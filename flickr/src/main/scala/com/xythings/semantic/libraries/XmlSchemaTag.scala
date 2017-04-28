package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._
/**
  * Create tags in the 'http://www.w3.org/2001/XMLSchema#' namespace
  */
object XmlSchemaTag  {
  val namespaceMapping = "http://www.w3.org/2001/XMLSchema#"->"xsd"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}
/**
  * A library of tags in the 'http://www.w3.org/2001/XMLSchema#' namespace
  */
object XmlSchemaTags {

  val STRING = XmlSchemaTag("string")
  val FLOAT = XmlSchemaTag("float")
  val DOUBLE = XmlSchemaTag("double")
  val DECIMAL = XmlSchemaTag("decimal")
  val INTEGER = XmlSchemaTag("integer")
  val INT = XmlSchemaTag("int")
  val DATE = XmlSchemaTag("date")

}

