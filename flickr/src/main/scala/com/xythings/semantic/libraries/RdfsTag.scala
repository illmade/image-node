package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._

object RdfsTag {
  val namespaceMapping = "http://www.w3.org/2000/01/rdf-schema#"->"rdfs"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}
/**
  * A library of tags in the 'http://www.w3.org/2000/01/rdf-schema#' namespace
  */
object RdfsTags {

  val COMMENT = RdfsTag("comment")
  val LABEL = RdfsTag("label")
  val SEE_ALSO = RdfsTag("seeAlso")
  val IS_DEFINED_BY = RdfsTag("isDefinedBy")
  val DOMAIN = RdfsTag("domain")
  val RANGE = RdfsTag("range")
  val DATATYPE = RdfsTag("Datatype")

}

