import tensorflow as tf

#
# We use a fixed cellsize to round x,y values into buckets
#
def create_heatmap():
    with tf.variable_scope('heatmap'):
        heatmap_scale = tf.placeholder(tf.float32, (1), name='scale')
        heatmap_threshold = tf.placeholder(tf.float32, (1), name='threshold')
        probabilities = tf.placeholder(tf.float32, (None,None,None,2), name='probabilities')
        bounds = tf.placeholder(tf.float32, (None,None,None,4), name='bounds')

        heat_scores = probabilities[0,:,:,1]
        heat_bounds = bounds[0,:,:,:]

        stride = tf.constant(2, dtype=tf.float32)
        cellsize = tf.constant(12, dtype=tf.float32)

        x1 = tf.transpose(heat_bounds[:,:,0])
        x2 = tf.transpose(heat_bounds[:,:,2])
        y1 = tf.transpose(heat_bounds[:,:,1])
        y2 = tf.transpose(heat_bounds[:,:,3])

        imap = tf.transpose(heat_scores)

        conditions = tf.greater(imap, heatmap_threshold)

        yx = tf.where(condition=conditions)
        y = yx[:,0]
        x = yx[:,1]
        score_heatmap = tf.gather_nd(imap, yx)

        x1_gather = tf.gather_nd(x1, yx)
        x2_gather = tf.gather_nd(x2, yx)
        y1_gather = tf.gather_nd(y1, yx)
        y2_gather = tf.gather_nd(y2, yx)

        stack = tf.stack([x1_gather, y1_gather, x2_gather, y2_gather])
        bounds_real = tf.transpose(stack, name='real_output')
        bb = tf.cast(yx, tf.float32)
        q1 = tf.round((bb*stride+1)/heatmap_scale)
        q2 = tf.round((bb*stride+cellsize+1)/heatmap_scale)
        expanded_scores = tf.expand_dims(tf.transpose(score_heatmap), 1)
        heatmap_output = tf.concat([q1, q2, expanded_scores, bounds_real], 1, name='bounds_output')
       
#
# tf now has a Non Maximal Surpression op but this works and is useful reference for tf.while_loop
#
def create_nms():
    with tf.variable_scope('nms'):
        nms_threshold = tf.placeholder(tf.float32, (1), 'threshold')
        s_bounds = tf.placeholder(tf.float32, (None,None), 'bounds')

        x1 = s_bounds[:,0]
        y1 = s_bounds[:,1]
        x2 = s_bounds[:,2]
        y2 = s_bounds[:,3]
        s = s_bounds[:,4]

        area = (x2-x1+1) * (y2-y1+1)

        number = tf.shape(s)[0]
        I = tf.nn.top_k(s, k=number, sorted=True)

        indices = I.indices

        pick = tf.zeros([number])

        counter = tf.constant(0)
        c = lambda counter_in, indices_in, pick_in: tf.greater(tf.shape(indices_in)[0], 0)

        def lamb(counter_in, indices_in, pick_in):

            i = indices_in[0]
            idx = indices_in[1:]

            i_c = tf.cast(i, tf.float32)

            pick_add = tf.scatter_nd([[counter_in]], [i_c], tf.shape(pick_in))
            pick_out = tf.add(pick_in, pick_add, name="pick")

            x1i = x1[i]
            x1ix = tf.gather(x1, idx)

            x1d_gather = tf.gather(x1, idx)

            xx1 = tf.maximum(x1[i],  tf.gather(x1, idx))
            yy1 = tf.maximum(y1[i],  tf.gather(y1, idx))
            xx2 = tf.minimum(x2[i],  tf.gather(x2, idx))
            yy2 = tf.minimum(y2[i],  tf.gather(y2, idx))
            w = tf.maximum(0.0, xx2-xx1+1)
            h = tf.maximum(0.0, yy2-yy1+1)

            inter = w * h
            o_area = tf.gather(area, idx)
            overlap = inter / (area[i] + o_area - inter)

            nms_condition = tf.less_equal(overlap, nms_threshold)
            ii = tf.where(condition=nms_condition)

            indices_out = tf.reshape(tf.gather(idx, ii), (-1,), name="indices")

            out_length = tf.shape(indices_out)

            return tf.add(counter_in, 1, name="counter"), indices_out, pick_out

        as_list = tf.while_loop(c, lamb, [counter, indices, pick], back_prop=False)
        
        pick_out = tf.cast(as_list[2], tf.int32)
        nms_pick = tf.multiply(pick_out[0:as_list[0]], 1, name="output")