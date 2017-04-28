package com.xythings.xml

/**
  * Created by tim on 28/04/2017.
  */
import scala.xml.Elem
import scala.xml.Node
import scala.xml.Text
import scala.xml.XML

import com.xythings.semantic._

object RdfReader {

  def apply(rawXml : Elem) = new RdfReader(rawXml)

  def apply(inputStream : java.io.InputStream) = new RdfReader(XML.load(inputStream))

  def apply(file : String): RdfReader = {
    val fileInputStream = new java.io.FileInputStream(file)
    new RdfReader(XML.load(fileInputStream))
  }
}
/**
  * RdfReader a simple way to read RDF in the format where all resources start
  * with <rdf:Description>
  */
class RdfReader(
                 rawXml : Elem,
                 tagReader : TagReader = TagReader(None),
                 trim : Boolean = true) extends Iterable[Edge] {

  val logger = java.util.logging.Logger.getLogger("RdfReader")
  //Both rdf and rdfs seem to be used for datatype...
  private val rdfDatatype = Tag("http://www.w3.org/1999/02/22-rdf-syntax-ns#","datatype")
  private val rdfsDatatype = Tag("http://www.w3.org/2000/01/rdf-schema#","Datatype")

  private val xml = if (trim)
    scala.xml.Utility.trim(rawXml)
  else
    rawXml

  def iterator = {

    val descriptions = xml \\ "Description"

    var elements : List[Edge] = Nil

    for (member <- descriptions){

      val subjectTag : Option[Tag] = attribute(member, libraries.RdfTags.ABOUT) match {
        case None =>
          logger.warning("Resource does not have about attribute" + member)
          None
        case some =>
          val head = some.get.head
          head match {
            case text : Text =>
              textToTag(text)
            case other =>
              logger.warning("Resource content should be text, instead: " + other)
              None
          }
      }

      val tripleParts = member.child

      for (part <- tripleParts){
        part match {
          case text : Text =>
            val data = text.data.trim
            if (!data.matches(""))
              logger.warning("Resource should have xml elem content not: " + text)
          case elem : Elem =>
            subjectTag match {
              case None =>

              case some =>
                elemEdge(some.get, elem) match {
                  case None =>
                  //we failed to create an edge
                  case newEdge =>
                    elements = newEdge.get :: elements
                }

            }
        }
      }
    }
    elements.iterator
  }

  /**
    * Each edge has a subjectTag, predicateTag and element data
    */
  private def elemEdge(subjectTag : Tag, elem : Elem) : Option[Edge] = {
    if(elem.namespace==null) //This happens sometimes for no obvious reason
      logger.warning("elem has null namespace: " + elem)

    val predicateTag = Tag(elem.namespace, elem.label)

    val lang : Option[String] = elem.attribute("http://www.w3.org/XML/1998/namespace", "lang") match {
      case None =>
        None
      case some =>
        new Some("@" + some.get)
    }
    /**
      * We apply xsd datatypes to objects
      */
    val datatype : Option[Tag] = elem.attribute(rdfsDatatype.namespace, rdfsDatatype.localName) match {
      case None => //check for rdf:datatype
        elem.attribute(rdfDatatype.namespace, rdfDatatype.localName) match {
      case None =>
        None
      case some =>
        new Some(Tag(some.get.text))
      }
      case some =>
        val tagUri = some.get.text
        new Some(Tag(tagUri))
    }
    val readTag : Option[Tag] = elem.attribute(libraries.RdfTags.RESOURCE.namespace, libraries.RdfTags.RESOURCE.localName) match {
      case None =>
        //need to alert error in rdf if no elem.child - then exit
        if(elem.child.isEmpty)
          logger.warning("Elem must have child: " + elem)

        val imageElem = elem.child.head

        tagReader.readTag(imageElem.text, datatype)
      case some =>
        val tagUri = some.get.text
        Some(Tag(tagUri))
    }
    readTag match {
      case None =>
        logger.warning("Could not create edge: " )
        None
      case some =>
        Some(Edge(subjectTag, predicateTag, some.get))
    }
  }

  /**
    * Check our text has an url as content, if not warn
    */
  private def textToTag(text : Text) : Option[Tag] = {
    val data = text.data
    if (data.startsWith("http"))
      Some(Tag(data))
    else{
      logger.warning("xml data is not url - cannot create a Tag " + data)
      None
    }
  }

  /**
    * Returns Option[Seq[Node]] for attributes matching the given tag
    */
  private def attribute(elem : Node, tag : Tag) : Option[Seq[Node]] = elem.attribute(tag.namespace, tag.localName)

}