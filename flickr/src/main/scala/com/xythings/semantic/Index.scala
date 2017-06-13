package com.xythings.semantic

/**
  * Created by tim on 28/04/2017.
  * To do more with edges we need to index them where:
  * the index is itself an Edge
  * an index uses Wildcard Tags to form collections
  * e.g
  * An index for a resource (a collection of edges with the same identifier) would be:
  * (identifier, Wildcard, Wildcard)
  *
  * These objects are just convenient ways of creating index edges
  */
trait IndexFactory {
  /**
    * Form the index version of the given Edge
    */
  def index(edge : Edge) : Edge

}
/**
  * A tag which represents a Wildcard
  */
object Wildcard extends Tag {
  val namespace = "_"
  val localName = "_"
}
/**
  * A collection of edges can then be queried by an index Edge
  * - We return an Option for better representation of remote graphs
  */
trait EdgeQuery {
  /**
    * Each index Edge matches an Iterable[Edge] or None
    */
  def query(index : Edge) : Option[Iterable[Edge]]

}

/**
  * Index by Edge identifier
  */
object ResourceIndex extends IndexFactory {
  /**
    * Implement IndexFactory
    */
  def index(edge : Edge) : Edge = make(edge.identifier)

  private def make(resource : Tag) = Edge(resource, Wildcard, Wildcard)
  /**
    * Simplify index creation
    */
  def apply(resource : Tag) : Edge = make(resource)

}

/**
  * Index by Edge identifier and property
  * The ResourcePropertyIndex also works as a functional index
  */
object ResourcePropertyIndex extends IndexFactory {
  /**
    * Implement IndexFactory
    */
  def index(edge : Edge) : Edge = make(edge.identifier, edge.property)

  private def make(resource : Tag, property : Tag) = Edge(resource, property, Wildcard)
  /**
    * Simplify index creation
    */
  def apply(resource : Tag, property : Tag) : Edge = make(resource, property)

}

/**
  * Index by Edge property
  */
object PropertyIndex extends IndexFactory {
  /**
    * Implement IndexFactory
    */
  def index(edge : Edge) : Edge = make(edge.property)

  private def make(property : Tag) : Edge = Edge(Wildcard, property, Wildcard)
  /**
    * Simplify index creation
    */
  def apply(property : Tag) : Edge = make(property)

}
/**
  * Index by Edge property and _3
  */
object PropertyObjectIndex extends IndexFactory {
  /**
    * Implement IndexFactory
    */
  def index(edge : Edge) : Edge = make(edge.property, edge._3)

  private def make(property : Tag, _3 : Tag) = Edge(Wildcard, property, _3)
  /**
    * Simplify index creation
    */
  def apply(property : Tag, _3 : Tag) : Edge = make(property, _3)

}

/**
  * An index by Edge _3
  */
object ObjectIndex extends IndexFactory {
  /**
    * Implement IndexFactory
    */
  def index(edge : Edge) : Edge = make(edge._3)

  private def make(image : Tag) = Edge(Wildcard, Wildcard, image)
  /**
    * Simplify index creation
    */
  def apply(_3 : Tag) : Edge = make(_3)

}
