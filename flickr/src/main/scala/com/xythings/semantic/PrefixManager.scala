package com.xythings.semantic

/**
  * Created by tim on 28/04/2017.
  *
  * In common with other namespace libraries Semantic maps namespaces to prefixes
  * to improve legibility and compactness
  */
trait PrefixManager {
  /**
    * Return a prefix string for a Tags namespace if available
    */
  def prefixFor(tag : Tag) : Option[String]
  /**
    * Given a prefix return a namespace if available
    */
  def namespaceFor(prefix : String) : Option[String]
}
/**
  * A simple Map[String, String] based PrefixManager
  */
class SimplePrefixManager(namespaceToPrefixMap : Map[String, String]) extends PrefixManager {
  //Set up the internal prefix to namespace map
  private val prefixToNamespaceMap = namespaceToPrefixMap.map(elem => elem._2 -> elem._1)
  /**
    * Return a prefix string for a Tags namespace if available
    */
  def prefixFor(tag : Tag) : Option[String] = namespaceToPrefixMap.get(tag.namespace)
  /**
    * Given a prefix return a namespace if available
    */
  def namespaceFor(prefix : String) : Option[String] = prefixToNamespaceMap.get(prefix)

}
/**
  * Set up a default prefixManager to handle most basic situations
  */
object PrefixManager {

  val logger = java.util.logging.Logger.getLogger("PrefixManager")

  private val defaultNamespacePrefixMap = Map[String, String](
    libraries.RdfTag.namespaceMapping,
    libraries.XmlSchemaTag.namespaceMapping,
    libraries.RdfsTag.namespaceMapping,
    libraries.WgsTag.namespaceMapping,
    libraries.DcmiTag.namespaceMapping,
    libraries.SkosTag.namespaceMapping,
    libraries.XyGeometryTag.namespaceMapping
//    libraries.FoafTag.namespaceMapping,
//    libraries.OwlTag.namespaceMapping,
//    libraries.XyDemographicsTag.namespaceMapping,
//    libraries.XyUserTag.namespaceMapping,
//    libraries.XyTableTag.namespaceMapping
  )

  def apply(namespacePrefixMap : Option[Map[String, String]]) : PrefixManager = {
    namespacePrefixMap match {
      case None =>
        new SimplePrefixManager(defaultNamespacePrefixMap)
      case some =>
        new SimplePrefixManager(defaultNamespacePrefixMap ++ some.get)
    }
  }
  /**
    * Look for prefix properties - map the namespace of the identifier to the localName of the _3
    */
  def apply(prefixGraph : EdgeQuery) : PrefixManager = {
    val namespaceToPrefixMap : Map[String, String] = prefixGraph.query(PropertyIndex(libraries.XmlTags.PREFIX)) match {
      case None =>
        logger.warning("No prefix mappings, in graph")
        Map.empty
      case some =>
        val entries = some.get.map(edge => edge._1.namespace -> edge._3.localName)
        entries.toMap
    }
    new SimplePrefixManager(defaultNamespacePrefixMap ++ namespaceToPrefixMap)
  }

}