package com.xythings.geometry.index

/**
  * Created by tim on 28/04/2017.
  */
/**
  * A trait for indexing shapes using grid indexes.
  * i.e
  * a point maps to an index which represents a square shape
  * a point maps to a 'pixel'
  */
trait GridIndexer extends com.xythings.geometry.ProjectionR2 {

  val gridSize : Int
  /**
    * For the given shape return all the indexes as points representing the
    * starting corner for a box of gridSize
    */
  def indexShape(points : Iterable[(Double, Double)]) : Iterable[(Int, Int)]
  /**
    * Return the index for a shape
    */
  def indexPoint(point : (Double, Double)) : (Int, Int) = {
    val mapping = project(point)
    (indexValue(mapping._1), indexValue(mapping._2))
  }

  def indexValue(value : Double) = (value/gridSize).floor.toInt

}
