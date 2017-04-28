package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._
/**
  * Create tags in the 'http://data.xythings.com/namespace/geometry/' namespace
  */
object XyGeometryTag  {
  val namespaceMapping = "http://data.xythings.com/namespace/geometry/"->"geometry"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}
/**
  * A library of tags in the 'http://data.xythings.com/namespace/geometry/' namespace
  */
object XyGeometryTags {

  val DISTANCE = XyGeometryTag("distance")
  val ORIGIN = XyGeometryTag("origin")
  val BOUNDS = XyGeometryTag("bounds")
  val HULL = XyGeometryTag("hull")
  val WALK = XyGeometryTag("walk")
  val LINEAR_RING = XyGeometryTag("linearRing")
  val SHAPE = XyGeometryTag("shape")
  val POLYGON = XyGeometryTag("polygon")
  val MULTI_POLYGON = XyGeometryTag("multiPolygon")
  val EXTERIOR = XyGeometryTag("exterior")
  val NEARBY = XyGeometryTag("nearby")
  val CLOSEST = XyGeometryTag("closest")
  val NEIGHBOUR = XyGeometryTag("neighbour")
  val CONTAINS = XyGeometryTag("contains")
  val CONTAINED_BY = XyGeometryTag("containedBy")
  val CENTROID = XyGeometryTag("centroid")
  /**
    * The following tags are used to create object tags with
    * specific serializations and uses
    */
  val POINT_R2 = XyGeometryTag("pointR2")
  //POINTS_R2 can be a walk, linear ring, simple polygon etc
  val POINTS_R2 = XyGeometryTag("pointsR2")
  //SEQUENCE_POINTS_R2 can be a complex polygon or a multi-polygon of simple polygons (no interiors)
  val SEQUENCE_POINTS_R2 = XyGeometryTag("sequencePointsR2")
  //ARRAY_SEQUENCE_POINTS_R2 can be a multi-polygon of complex polygons (with interiors)
  val ARRAY_SEQUENCE_POINTS_R2 = XyGeometryTag("arraySequencePointsR2")

}