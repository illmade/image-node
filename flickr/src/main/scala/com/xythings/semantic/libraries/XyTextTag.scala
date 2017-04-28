package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._
/**
  * Create tags in the 'http://data.xythings.com/namespace/geometry/' namespace
  */
object XyTextTag  {
  val namespaceMapping = "http://data.xythings.com/namespace/text/"->"text"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}
/**
  * A library of tags in the "http://data.xythings.com/namespace/text/" namespace
  */
object XyTextTags {

  val STRING_SEQUENCE = XyTextTag("stringSequence")
  val STRING_NORMALIZATION = XyTextTag("stringNormalization")
  val FRAGMENT = XyTextTag("fragment")
  val FRAGMENT_OF = XyTextTag("fragmentOf")

}

