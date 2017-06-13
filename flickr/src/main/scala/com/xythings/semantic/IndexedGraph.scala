package com.xythings.semantic

/**
  * Created by tim on 28/04/2017.
  *
  * Combining EdgeStore with EdgeQuery gives us an IndexedGraph
  */
trait IndexedGraph extends EditableEdgeCollection with EdgeQuery {

  def + (edge : Edge) = add(edge)

}
/**
  * Hold apply constructors that use SimpleIndexedGraph
  */
object IndexedGraph {
  /**
    * create an indexedGraph with the default PrefixManager
    */
  def apply(inputEdges : Iterable[Edge], indexes : Iterable[IndexFactory]) =
    new SimpleIndexedGraph(inputEdges, indexes, PrefixManager(None))
  /**
    * create an indexedGraph with the default Indexes: List(ResourceIndex, PropertyIndex, ResourcePropertyIndex, ObjectIndex, PropertyObjectIndex)
    */
  def apply(inputEdges : Iterable[Edge]): IndexedGraph = {
    val indexes = List(ResourceIndex, PropertyIndex, ResourcePropertyIndex, ObjectIndex, PropertyObjectIndex)
    new SimpleIndexedGraph(inputEdges, indexes, PrefixManager(None))
  }
}
/**
  * Implement EdgeStore and EdgeQuery
  */
class SimpleIndexedGraph(inputEdges : Iterable[Edge], indexes : Iterable[IndexFactory], prefixManager : PrefixManager)
    extends IndexedGraph {

  private val logger = java.util.logging.Logger.getLogger("SimpleIndexedGraph")
  private val edgeStringifier = new SmallEdgeStringifier(prefixManager)
  private var edgeMap : Map[String, List[Edge]] = Map.empty

  inputEdges.foreach(edge => add(edge))
  /**
    * For each indexFactory create a index version of the edge -
    * this index is stringified and used as a key for the internal edgeMap
    */
  def add(edge : Edge): Unit = {
    for (indexer <- indexes){
      val key = edgeStringifier.stringify(indexer.index(edge))
      edgeMap.get(key) match {
        case None =>
          edgeMap += key -> List(edge)
        case some =>
          edgeMap += key -> (edge :: some.get)
      }
    }
  }

  def delete(edge : Edge): Unit = {
    for(indexer <- indexes){
      val key = edgeStringifier.stringify(indexer.index(edge))
      edgeMap.get(key) match {
        case None =>
          logger.warning(s"attempted to remove absent key $key")
        case some =>
          edgeMap += key -> some.get.filterNot(existing => existing.equals(edge))
      }
    }
  }

  def query(index : Edge) : Option[Iterable[Edge]] = edgeMap.get(edgeStringifier.stringify(index))

}
