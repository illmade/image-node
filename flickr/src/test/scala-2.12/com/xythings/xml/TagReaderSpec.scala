package com.xythings.xml

import com.xythings.semantic.{ObjectTag, libraries}
import org.specs2.mutable.Specification

/**
  * Created by tim on 30/05/2017.
  */
class TagReaderSpec extends Specification {

  "test Spec for TagReader".txt

  val basicTagReader = TagReader(None)

  val typeTag = ObjectTag("http://www.w3.org/2001/XMLSchema/double/", 2.0)
  val textTag = ObjectTag("http://www.w3.org/2001/XMLSchema/string/", 2.0)

  "basicTagReader should " >> {
    "generate an objectTag for 2.0 when given a mapping" >> {
      basicTagReader.readTag("2.0", Some(libraries.XmlSchemaTags.DOUBLE)) must beSome(typeTag)
    }
  }
  "basicTagReader should " >> {
    "generate an objectTag for 2.0 but with type string when no mapping" >> {
      basicTagReader.readTag("2.0", None) must beSome(textTag)
    }
  }

}
