package com.xythings.data.flickr

/**
  * Created by tim on 27/04/2017.
  */
import com.xythings.xml._

class PhotoDetail(val latitudeLongitude : (Double, Double),
                  val views : Int,
                  val canDownload : Int,
                  val owner : String,
                  val secret : String,
                  val url : String,
                  val tags : Iterable[String]) {

  override def toString = s"$latitudeLongitude, $views, $owner, $url"

}

class PhotoReader(key: LocationApiKey, xmlReader : RemoteXmlReader = new HttpXmlReader) {

  val logger = java.util.logging.Logger.getLogger("PhotoDetail")

  val flickrRest = new FlickrRest(key)

  def getDetail(id : String) : Option[PhotoDetail] = {
    logger.info(s"Getting photo: ${id}")

    val restUrl = flickrRest.photoInfo(id)

    try {
      val response = xmlReader.getXml(restUrl)

      response match {
        case None =>
          logger.warning("Response does not contain xml")
          None
        case responseXml =>
          val photoElement = responseXml.get \ "photo"

          if((responseXml.get \ "@stat").text == "ok"){
            try {
              val photoLocation = photoElement \  "location"

              val latitudeLongitude = ((photoLocation \ "@latitude").text.toDouble, (photoLocation \ "@longitude").text.toDouble)
              val owner = photoElement \ "owner"
              val ownerId = (owner \ "@nsid").text
              val views = (photoElement \ "@views").text.toInt

              val tags = (photoElement \ "tags" text) split("\\W").trim

              val farm = photoElement \ "@farm"
              val server = photoElement \ "@server"
              val secret = photoElement \ "@secret"

              val usage = photoElement \ "usage"
              val canDownload = (usage \ "@candownload").text.toInt

              val urls = (photoElement \\ "url").filter(node => node.attribute("type")
                .exists(urlType => urlType.text == "photopage"))

              val photoSecret = s"http://farm${farm}.staticflickr.com/${server}/${id}_${secret}.jpg"

              val photoUrl = if(!urls.isEmpty)urls.head.text
              else photoSecret

              new Some(new PhotoDetail(latitudeLongitude, views, canDownload, ownerId, photoSecret, photoUrl,
                tags.filter(a => if(a=="")false; else true)))
            }
            catch {
              case e : Exception =>
                None
            }
          }
          else {
            logger.warning("Failed response from server for: " + id)
            None
          }
      }
    }
    catch {
      case e : Exception =>
        None
    }

  }
}
