package com.xythings.geometry

/**
  * Created by tim on 28/04/2017.
  */
trait ProjectionR2 {

  def project(pointR2 : (Double, Double)) : (Double, Double)

}
