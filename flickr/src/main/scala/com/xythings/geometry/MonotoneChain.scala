package com.xythings.geometry

/**
  * Created by tim on 28/04/2017.
  *
  * A method of calculating the convex hull on a sphere of a finite set of points in R2
  *
  * see: https://en.wikipedia.org/wiki/MonotoneChain
  */
object MonotoneChain {
  /**
    * We force sorting of points to be ordered by their natural ordering
    */
  def apply(points : Iterable[(Double, Double)]): List[(Double, Double)] = {
    val sortedPoints = points.toList.sortBy(x => x)
    findHull(sortedPoints)
  }
  /**
    * Given a list of points (sorted by their natural ordering) return a convex hull
    *
    * - note that different coordinate systems may require a non default ScanClockR2
    * to accurately determining clockwise turns
    */
  private def findHull(
                        orderedPoints : List[(Double, Double)],
                        scanClockR2 : ScanClockR2 = SimpleClock) : List[(Double, Double)] = {
    var lower : List[(Double, Double)] = Nil
    var reverse : List[(Double, Double)] = Nil

    for (point <- orderedPoints){
      while (lower.length >= 2 && scanClockR2.isClockwise(lower(1), lower.head, point) )
        lower = lower.tail
      lower = point :: lower
      reverse = point :: reverse
    }

    var upper : List[(Double, Double)] = Nil

    for (point <- reverse){
      while (upper.length >= 2 && scanClockR2.isClockwise(upper(1), upper.head, point) )
        upper = upper.tail
      upper = point :: upper
    }

    lower ++ upper
  }
}
/**
  * Given 3 points determine if they describe a clockwise turn a->b->c
  */
trait ScanClockR2 {

  def isClockwise(
                   pointA : (Double, Double),
                   pointB : (Double, Double),
                   pointC : (Double, Double)) : Boolean

}
/**
  * Simple implementation of ScanClockR2
  */
object SimpleClock extends ScanClockR2 {
  /**
    * Implement ScanClockR2
    */
  def isClockwise(
                   pointA : (Double, Double),
                   pointB : (Double, Double),
                   pointC : (Double, Double)) : Boolean = {
    val simpleCross : Double = (pointB._1 - pointA._1)*(pointC._2 - pointA._2) - (pointB._2 - pointA._2)*(pointC._1 - pointA._1)

    if(simpleCross <= 0)
      true
    else
      false
  }

  import scala.annotation.tailrec
  @tailrec
  def wind(points : Iterable[(Double, Double)], winding : Double = 0) : Double = {

    if (!points.tail.isEmpty){
      val tail = points.tail
      val newWinding = winding + ((points.head._1 + tail.head._1) * (points.head._2 - tail.head._2))
      wind(tail, newWinding)
    }
    else {
      winding
    }

  }

}
