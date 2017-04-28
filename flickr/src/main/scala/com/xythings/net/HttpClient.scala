package com.xythings.net

import org.apache.http.client.HttpClient
import org.apache.http.impl.client._
import org.apache.http.impl.conn._
/**
  * Created by tim on 27/04/2017.
  */
trait DefaultClient {

  def instance: HttpClient

}
/**
  * Set up a semantic browser client
  */
object SimpleHttpClient extends DefaultClient {

  def instance: CloseableHttpClient = {
    val builder = HttpClientBuilder.create()
    val connectionManager = new BasicHttpClientConnectionManager()
    builder.setRedirectStrategy(new DefaultRedirectStrategy).setUserAgent("XmlBrowser/1.6")

    builder.build
  }

}