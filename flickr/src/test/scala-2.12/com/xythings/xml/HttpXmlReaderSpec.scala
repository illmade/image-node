package com.xythings.xml

import org.specs2.mutable.Specification

/**
  * Created by tim on 30/05/2017.
  */
class HttpXmlReaderSpec extends Specification {

  "test Spec for HttpXmlReader".txt

  val reader = new HttpXmlReader

  val localXml = reader.getXml("http://localhost:63342/flickr/com.xythings.xml/test.xml")

  "reader should" >> {
    "give valid _ for _ " >> {
      localXml mustEqual None
    }
  }

}
