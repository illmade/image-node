package com.xythings.xml

import org.apache.http.client.HttpClient
import org.apache.http.client.methods.HttpGet

import scala.xml._
/**
  * Created by tim on 27/04/2017.
  */

trait RemoteXmlReader {

  def getXml(xmlUrl: String): Option[Elem]

}

class HttpXmlReader(httpClient: HttpClient = com.xythings.net.SimpleHttpClient.instance)
      extends RemoteXmlReader{

  val logger = java.util.logging.Logger.getLogger("HttpXmlReader")

  def getXml(xmlUrl: String): Option[Elem] = {

    val get = new HttpGet(xmlUrl)

    val response = httpClient.execute(get)

    val httpEntity = response.getEntity

    response.getStatusLine.getStatusCode match {
      case 200 =>
        val content = httpEntity.getContent
        new Some(XML.load(content))
      case other =>
        logger.warning(s"Client returned ${other} for ${xmlUrl}")
        None
    }

  }

}
