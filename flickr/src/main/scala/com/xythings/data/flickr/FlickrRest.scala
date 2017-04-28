package com.xythings.data.flickr

import com.xythings.semantic._
/**
  * Created by tim on 27/04/2017.
  */

trait ApiKey {

  def getKey : String

}

class LocationApiKey(val getKey : String) extends ApiKey

object PlaceMaker {

  def photoInfo(photoId : String, photoSecret : String, apiKey : ApiKey, format : String = "rest") = s"https://api.flickr.com/services/rest/?method=flickr.photos.getInfo&api_key=${apiKey.getKey}&photo_id=$photoId&secret=$photoSecret&format=$format"

}

object FickrRest {

  val header = "https://api.flickr.com/services/rest/?method="

  /**
    * bbox The 4 values represent the bottom-left corner of the box and the top-right corner, minimum_longitude, minimum_latitude, maximum_longitude, maximum_latitude
    */
  def bbox(bounds : ((Double, Double), (Double, Double))) : String = s"${bounds._1._2},${bounds._1._1},${bounds._2._2},${bounds._2._1}"

}

class FlickrRest(apiKey : ApiKey) {

  import FickrRest._

  val logger = java.util.logging.Logger.getLogger("FlickrRest")

  def photoLocation(photoId : String, format : String = "rest") = s"${header}flickr.photos.geo.getLocation&api_key=${apiKey.getKey}&photo_id=$photoId&format=$format"

  def photoInfo(photoId : String, format : String = "rest") = {
    s"${header}flickr.photos.getInfo&api_key=${apiKey.getKey}&photo_id=$photoId&format=$format"
  }

  def findPhotos(bounds : ((Double, Double), (Double, Double)), place : Option[String],
                 page : Int = 1, user : Option[String] = None) : String = {
    val encodedPlace = place match {
      case None =>
        ""
      case some =>
        "&tags=" + java.net.URLEncoder.encode(some.get, "UTF-8")
    }

    val url = s"${header}flickr.photos.search&api_key=${apiKey.getKey}&bbox=${bbox(bounds)}&page=$page$encodedPlace"
    logger.info(s"Looking up url $url")
    user match {
      case None =>
        url
      case some =>
        s"$url&user_id=${some.get}"
    }
  }
}
