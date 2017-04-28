package com.xythings.semantic

/**
  * Created by tim on 28/04/2017.
  */
object ObjectTools {
  /**
    * For a doubleTag we just grab the local value
    */
  def getDouble(tag : Tag) : Option[Double] = {
    tag match {
      case objectTag : ObjectTag =>
        objectTag.local match {
          case double : Double =>
            Some(double)
          case _ =>
            None
        }
      case _ =>
        None
    }
  }
  /**
    * For a floatTag we just grab the local value
    */
  def getFloat(tag : Tag) : Option[Float] = {
    tag match {
      case objectTag : ObjectTag =>
        objectTag.local match {
          case float : Float =>
            Some(float)
          case _ =>
            None
        }
      case _ =>
        None
    }
  }
  /**
    * For an Int Tag we just grab the local value
    */
  def getInt(tag : Tag) : Option[Int] = {
    tag match {
      case objectTag : ObjectTag =>
        objectTag.local match {
          case integer : Int =>
            Some(integer)
          case _ =>
            None
        }
      case _ =>
        None
    }
  }
  /**
    * This is perhaps not the most elegant but we attempt through process to make it stable
    * Only return for tags with the namespace http://data.ordnancesurvey.co.uk/ontology/geometry/pointR2/
    */
  def getPoint(tag : Tag) : Option[(Double, Double)] = {
    tag.namespace.dropRight(1) match {
      case libraries.XyGeometryTags.POINT_R2.url =>
        tag match {
          case objectTag : ObjectTag =>
            objectTag.local match {
              case point : (Any, Any) =>
                Some(point.asInstanceOf[(Double, Double)])
              case _ =>
                None
            }
          case _ =>
            None
        }
      case _ =>
        None
    }
  }
  /**
    * This is perhaps not the most elegant but we attempt through process to make it stable
    */
  def getPoints(tag : Tag) : Option[Iterable[(Double, Double)]] = {
    tag.namespace.dropRight(1) match {
      case libraries.XyGeometryTags.POINTS_R2.url =>
        tag match {
          case objectTag : ObjectTag =>
            objectTag.local match {
              case points : Iterable[Any] =>
                Some(points.asInstanceOf[Iterable[(Double, Double)]])
              case _ =>
                None
            }
          case _ =>
            None
        }
      case _ =>
        None
    }
  }

  /**
    * SequencePoints may represent a polygon with holes (the first Iterable being the
    * outer ring, all subsequent Iterables represent holes) or a sequence of simple polygons
    * depending on the edges property
    */
  def getSequencePoints(tag : Tag) : Option[Iterable[Iterable[(Double, Double)]]] = {
    tag.namespace.dropRight(1) match {
      case libraries.XyGeometryTags.SEQUENCE_POINTS_R2.url =>
        tag match {
          case objectTag : ObjectTag =>
            objectTag.local match {
              case multiPoints : Iterable[Any] =>
                Some(multiPoints.map(points => {
                  points match {
                    case path : Iterable[Any] =>
                      path.asInstanceOf[Iterable[(Double, Double)]]
                  }
                }))
              case _ =>
                None
            }
          case _ =>
            None
        }
      case _ =>
        None
    }
  }
}
