package com.xythings.semantic.impl

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.xml._
import com.xythings.semantic._
/**
  * PrefixManagerImpl will at least return the default prefixManager but
  * will attempt to create mappings from the resource file defaultPrefix.rdf
  */
object PrefixManagerImpl  {

  val logger = java.util.logging.Logger.getLogger("PrefixManagerImpl")

  def apply(prefixEdges : Option[Iterable[Edge]]) : PrefixManager = {
    prefixEdges match {
      case None =>
        try {
          val prefixStream = PrefixManagerImpl.getClass.getResourceAsStream("defaultPrefix.rdf")
          val edges = RdfReader(prefixStream)
          PrefixManager(IndexedGraph(edges))
        }
        catch {
          case e : Exception =>
            logger.warning("Could not form prefix graph, missing file 'defaultPrefix.rdf; from resources?")
            PrefixManager(None)
        }
      case some =>
        PrefixManager(IndexedGraph(some.get))
    }
  }
}