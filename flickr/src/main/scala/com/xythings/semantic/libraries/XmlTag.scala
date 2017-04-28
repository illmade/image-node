package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._
/**
  * Note the strange non-standard mapping
  */
object XmlTag {
  val namespaceMapping = "http://www.w3.org/XML/1998/namespace" -> "xml"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}
/**
  * A library of tags in the "http://www.w3.org/XML/1998/namespace" namespace
  */
object XmlTags {

  val PREFIX = XmlTag("prefix")

}

