package com.xythings.xml

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._

import scala.xml._
/**
  * Make writing to strings and files easy
  */
object RdfWriter {

  def apply(filename : String, edges : Iterable[Edge]) {
    save(filename, edges)
  }
  /**
    * Use a fileWriter and nio to output a file
    */
  def save(filename : String, edges : Iterable[Edge]) {
    val tripleWriter = new RdfWriter

    val encoding: String = "UTF-8"

    val fileOutputStream = new java.io.FileOutputStream(filename)
    val fileWriter = java.nio.channels.Channels.newWriter(fileOutputStream.getChannel, encoding)
    tripleWriter.write(fileWriter, edges, true, encoding)
    fileWriter.close
    fileOutputStream.close
  }
  /**
    * Create a StringWriter and output its result
    */
  def format(edges : Iterable[Edge]) : String = {
    val tripleWriter = new RdfWriter
    val xmlWriter = new java.io.StringWriter
    tripleWriter.write(xmlWriter, edges, true)
    xmlWriter.toString
  }
}
/**
  * Serialize triples in Rdf format
  */
class RdfWriter(
                 tagWriter : TagWriter = new SimpleTagWriter,
                 objectCaster : ObjectCaster = SimpleObjectCaster,
                 prefixManager : PrefixManager = impl.PrefixManagerImpl(None)) {

  private val logger = java.util.logging.Logger.getLogger("RdfWriter")

  def write(writer: java.io.Writer, edges : Iterable[Edge], xmlDecl: Boolean, encoding: String = "UTF-8") {

    if (xmlDecl)
      writer.write("<?xml version='1.0' encoding='" + encoding + "'?>\n")

    //Write our declarations and our xml namespace information
    val initializedXml = initializeXml(edges)

    writeHeader(writer, initializedXml._1, initializedXml._3)
    writeResources(writer, initializedXml._2, initializedXml._3)

    writer.append("\r</rdf:RDF>")
  }

  private def writeHeader(writer : java.io.Writer, entityMappings : Map[String, dtd.ParsedEntityDecl], namespaceToPrefixMappings : Map[String, String]){
    //We have finished the initial parse so we can write our declarations
    if (!entityMappings.isEmpty){
      writer.append("""<!DOCTYPE rdf:RDF [ """)
      writer.append("\n")
      for (entityDeclaration <- entityMappings){
        writer.append("\t")
        val stringBuilder = new StringBuilder
        entityDeclaration._2.buildString(stringBuilder)
        writer.append(stringBuilder.result)
        writer.append("\n")
      }
      writer.append("""]>""")
    }
    //And our namespace mappings
    writer.append("\n<rdf:RDF \n")
    for (mapping <- namespaceToPrefixMappings)
      writer.append("\t xmlns:" + mapping._2 + "='" + mapping._1 + "' \n")
    writer.append(">\n")

  }

  private def writeResources(writer : java.io.Writer, resources : Map[Tag, List[Edge]], namespaceToPrefixMappings : Map[String, String]) {
    //Our triples have been gathered into resources and missing namespace mappings created
    for (resource <- resources) {
      //Our urls are allowed unsafe XML strings
      val url = resource._1.namespace + TagWriter.safeName(resource._1.localName)

      writer.append("\n\t<rdf:Description rdf:about='" + url + "'>")

      for (edge <- resource._2){
        val prefix = namespaceToPrefixMappings(edge.property.namespace)

        val propertyString = prefix + ":" + TagWriter.safeName(edge.property.localName)

        writer.append("\n\t\t<" + propertyString)

        //We have the special case for XMLSchema/string/ where we don't want datatype messing things up
        if (edge._3.namespace == "http://www.w3.org/2001/XMLSchema/string/")
          writer.append(s""">${TagWriter.safeName(edge._3.localName)}</${propertyString}>""")
        else{
          val objectOption = tagWriter.writeTag(edge._3)
          objectOption._1 match {
            case None =>
              //close the property string
              writer.append(s""" rdf:resource="${edge._3.namespace}${objectOption._2}"/> """)
            case some =>
              //we have a datetype
              writer.append(s""" rdfs:Datatype="${some.get}">${objectOption._2}</${propertyString}>""")
          }
        }
      }
      writer.append("\n\t</rdf:Description>")
    }
  }

  /**
    * This creates doctype entities and xml namespace mappings
    * returns
    * _1 edges as resource bundles
    * _2 default namespace mappings for those missing from the prefix manager
    */
  private def initializeXml(edges : Iterable[Edge]) : (Map[String, dtd.ParsedEntityDecl], Map[Tag, List[Edge]], Map[String, String]) = {

    var unknownPrefixIndex = 0
    var entityMappings : Map[String, dtd.ParsedEntityDecl] = Map.empty

    //Ensure we have the RdfTag.namespaceMapping and the RdfsTag.namespaceMapping
    var namespaceToPrefixMappings : Map[String, String] = Map[String, String](
      libraries.RdfTag.namespaceMapping,
      libraries.RdfsTag.namespaceMapping)

    //Gather our resources together to tidy the output
    var resources : Map[Tag, List[Edge]] = Map.empty

    /**
      * Create a prefix mapping for unknown namespaces
      * - add known and unknown namespaces to the xmlns section of the document
      */
    for (edge <- edges) {
      resources.get(edge._1) match {
        case None =>
          resources += edge._1 -> List(edge)
        case some =>
          resources += edge._1 -> (edge :: some.get)
      }

      prefixManager.prefixFor(edge._2) match {
        case None =>
          //The prefix is unknown: create one and increment the unknownPrefixIndex
          val mapping = edge._2.namespace -> ("dfns" + unknownPrefixIndex)

          namespaceToPrefixMappings += mapping
          unknownPrefixIndex += 1
        case some =>
          namespaceToPrefixMappings += edge._2.namespace -> some.get
      }

      edge._3 match {
        case objectTag : ObjectTag =>
          objectCaster.getCast(edge._3.namespace) match {
            case None =>
              logger.warning(s"No cast for ${objectTag}, cannot create entityDeclaration")
            case castTag =>
              prefixManager.prefixFor(castTag.get) match {
                case None =>
                  //The prefix is unknown: create one and increment the unknownPrefixIndex
                  val mapping = edge._2.namespace -> ("dfns" + unknownPrefixIndex)

                  val entityDeclaration = new dtd.IntDef(mapping._1)
                  val parsedEntity = new dtd.ParsedEntityDecl(mapping._2, entityDeclaration)

                  entityMappings += mapping._1 -> parsedEntity
                  unknownPrefixIndex += 1
                case prefixOption =>
                  entityMappings.get(prefixOption.get) match {
                    case None =>
                      val entityDeclaration = new dtd.IntDef(castTag.get.namespace)
                      val parsedEntity = new dtd.ParsedEntityDecl(prefixOption.get, entityDeclaration)
                      val mapping = prefixOption.get -> parsedEntity
                      entityMappings += mapping
                    case _ =>
                  }
              }

          }
        case _ =>

      }

    }
    (entityMappings, resources, namespaceToPrefixMappings)
  }

}
