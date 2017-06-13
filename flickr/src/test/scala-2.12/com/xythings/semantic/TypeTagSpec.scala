package com.xythings.semantic

import org.specs2.mutable.Specification

/**
  * Created by tim on 30/05/2017.
  */
class TypeTagSpec extends Specification {
  "Test Spec for TypeTag".txt

  val intTag = TypeTag(libraries.XmlSchemaTags.INT, 1.0)

  "intTag should" >> {
    "have a namespace of libraries.XmlSchemaTags.INT without #" >> {
      intTag.namespace mustEqual libraries.XmlSchemaTags.INT.url.replaceAll("#", "/") + "/"
    }
  }

}
