package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._

object DcmiTag {
  val namespaceMapping = "http://purl.org/dc/terms/" -> "dcmi-terms"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}

object DcmiElementTag {
  val namespaceMapping = "http://purl.org/dc/elements/1.1/" -> "dcmi-element"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}

/**
  * A library of tags in the "http://purl.org/dc/terms/" namespace
  */
object DcmiTags {

  val IS_PART_OF = DcmiTag("isPartOf")
  val HAS_PART = DcmiTag("hasPart")
  val RIGHTS = DcmiTag("rights")
  val IMAGE = DcmiTag("image")
  val CREATOR = DcmiElementTag("creator")

}
