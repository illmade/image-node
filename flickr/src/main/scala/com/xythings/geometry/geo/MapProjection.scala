package com.xythings.geometry.geo

/**
  * Created by tim on 28/04/2017.
  */
trait MapProjection {

  def degreesToEastingNorthing(latitude : Double, longitude : Double) : (Double, Double)

  def eastingNorthingToDegrees(easting : Double, northing : Double) : (Double, Double)

}
