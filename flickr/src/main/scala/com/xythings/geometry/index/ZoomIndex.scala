package com.xythings.geometry.index
/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic.Tag
import com.xythings.geometry._
import com.xythings.geometry.raster._

object ZoomIndex {

  def indexTag(index : (Int, Int), zoom : Int, dataset : String = "geo") : Tag = Tag("http://data.xythings.com/namespace/index/geo/", s"$dataset$zoom$index")

}
/**
  * ZoomIndex creates a world level index based on values created by a raster.ZoomProjection(14)
  * for ZoomProjection the 'pixel' width of the world at a give zoom level is: 256.0 * pow(2, zoom)
  * - as can be seen the index is equivalent to a map tile, the index effectively describes
  * which geometries are present in a given tile.
  *
  * the raster/grid created has origin 0,0 in the top lefthand corner and an x,y order
  *
  * 0  1  2  3  4  5  6  7
  * 1 11 12 13 14 15 16 17
  * 2 21 22 23 24 25 26 27
  * 3 31 32 33 34 35 36...
  * 4 41 42 43 44 45 ...
  * 5 51 52 ...
  *
  * This is fully interoperable with the tiling system used in google maps
  *
  */
class ZoomIndex(projection : ProjectionR2 = new ZoomProjection(14)) extends GridIndexer {
  /**
    * our projections raster boxes are always 256 'pixels' wide
    */
  val gridSize : Int = 256

  private val logger = java.util.logging.Logger.getLogger("ZoomIndex")

  /**
    * We expect points in the format latitude,longitude
    */
  def indexShape(points : Iterable[(Double, Double)]) : Iterable[(Int, Int)] = {
    //These are in x,y format
    val path = rasterPath(points, projection)
    val pathBounds = path.getBounds2D

    var indexed : List[(Int, Int)] = Nil
    for (
      x <- indexValue(pathBounds.getMinX) to indexValue(pathBounds.getMaxX);
      y <- indexValue(pathBounds.getMinY) to indexValue(pathBounds.getMaxY)){

      val xPixel = gridSize * x
      val yPixel = gridSize * y

      if(path.intersects(xPixel, yPixel, gridSize, gridSize))
        indexed = (x, y) :: indexed
      else if(path.contains(xPixel, yPixel, gridSize, gridSize))
        indexed = (x, y) :: indexed
      else
        logger.info("excluded: " + x + "," + y)
    }
    indexed
  }

  def project(point : (Double, Double)) = projection.project(point._1, point._2)

  def rasterPath(points : Iterable[(Double, Double)], projection : com.xythings.geometry.ProjectionR2) : java.awt.geom.Path2D = {
    val path = new java.awt.geom.Path2D.Double
    val start = projection.project(points.head)
    path.moveTo(start._1, start._2)
    for(point <- points.tail){
      val moveTo = projection.project(point)
      path.lineTo(moveTo._1, moveTo._2)
    }
    path
  }
}
