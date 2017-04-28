package com.xythings.geometry.raster

/**
  * Created by tim on 28/04/2017.
  */
import scala.math._
import com.xythings.geometry.geo.MapProjection

trait GeoProjection extends com.xythings.geometry.ProjectionR2 {

  def pixelsToLatitudeLongitude(xy : (Double, Double)) : (Double, Double)

}
/**
  * A Map Projection where the projection changes as we change zoom levels
  * the base projection result is * pow(2, zoom) to give the final raster value
  * - This is a normal web (not transverse) Mercator projection
  */
class ZoomProjection(var zoom : Int = 10) extends GeoProjection with MapProjection {

  def degreesToEastingNorthing(latitude : Double, longitude : Double) : (Double, Double) = project(latitude, longitude)

  def eastingNorthingToDegrees(easting : Double, northing : Double) : (Double, Double) = pixelsToLatitudeLongitude(easting, northing)

  /**
    * Implement ProjectionR2
    * xy become latitudeLongitude in this projection
    * i.e do not enter longitudeLatitude to conform to xy
    * e.g a point on the greenwich meridian say (54.1, 0.0) is entered at such to
    * return a x,y co-ordinate: zoomProjection.project((54.1, 0.0)
    */
  def project(latitudeLongitude : (Double, Double)) : (Double, Double) = (pixelX(latitudeLongitude._2, zoom) ,pixelY(latitudeLongitude._1, zoom))

  private def pixelWidth(zoom : Int) : Double = 256.0 * pow(2, zoom)

  private def radianLength(zoom : Int) : Double =  pixelWidth(zoom) / (2.0 * Math.PI)

  private def worldX(longitude : Double) : Double = 128.0 + 256.0 / 360.0 * longitude

  private def worldY(latitude : Double) = 128.0 - gatanh(latitude.toRadians) * 256.0 / (2.0 * Math.PI)

  private def pixelX(longitude : Double, zoom : Int) : Double = worldX(longitude) * pow(2, zoom)

  private def pixelY(latitude : Double, zoom : Int) : Double = worldY(latitude) * pow(2, zoom)

  //pixelToLongitude is a mathematically equivalent formula to zoomLongitude
  private def pixelToLongitude(x: Double, zoom : Int) = (45*x/pow(2, zoom + 5)) - 180;

  private def gatanh(o : Double) : Double = log((1+sin(o))/cos(o))

  private def worldLatitude(y : Double) = (atan(sinh( (128.0 - y) * 2.0 * Math.PI/256.0 ))).toDegrees

  private def worldLongitude(x : Double) = (x - 128.0)  * 360.0 / 256.0

  def pixelsToLatitudeLongitude(xy : (Double, Double)) : (Double, Double) =
    (zoomLatitude(xy._2), zoomLongitude(xy._1))

  private def zoomLatitude(y : Double) = worldLatitude(y / pow(2, zoom))

  private def zoomLongitude(x : Double) = worldLongitude(x / pow(2, zoom))

}
