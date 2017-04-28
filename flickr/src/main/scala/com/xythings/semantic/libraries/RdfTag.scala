package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._

object RdfTag {
  val namespaceMapping = "http://www.w3.org/1999/02/22-rdf-syntax-ns#"->"rdf"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}
/**
  * A library of tags in the 'http://www.w3.org/1999/02/22-rdf-syntax-ns#' namespace
  */
object RdfTags {

  val ABOUT = RdfTag("about")
  val A = RdfTag("type")
  val RESOURCE = RdfTag("resource")
  val XML_LITERAL = RdfTag("XMLLiteral")
  val DESCRIPTION = RdfTag("Description")
  val RDF = RdfTag("RDF")
  val BAG = RdfTag("bag")
  val SEQUENCE = RdfTag("Seq")
  val PREDICATE =  RdfTag("predicate")
  val SUBJECT =  RdfTag("subject")
  val OBJECT =  RdfTag("object")
  val LIST = RdfTag("List")
  val NIL = RdfTag("nil")
  val REST = RdfTag("rest")
  val FIRST = RdfTag("first")
  val _1 = RdfTag("_1")
  val _2 = RdfTag("_2")
  val _3 = RdfTag("_3")

}
