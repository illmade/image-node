package com.xythings.semantic

import scala.math._
/**
  * A graph is at its core a collection of edges:
  * Where each edge is two vertices _1, _3 connected by a property _2
  * An edge is generally seen to be directed with the _1 vertex addressable as the
  * edges identifier.
  * A subgraph where all edges have the same identifier is refered to as a resource
  */
class Edge(val identifier : Tag, val property : Tag, val target : Tag) {

  val _1: Tag = identifier
  val _2: Tag = property
  val _3: Tag = target
  /**
    * The hashCode is based on _1, _2, _3
    */
  override def hashCode = 23 ^ _1.hashCode ^ _2.hashCode ^ _3.hashCode
  /**
    * An Edge is the same as another if _1, _2, _3 are equal
    * as it is more likely the object is the diffence we test it first
    */
  override def equals(any : Any) : Boolean = {
    any match {
      case that : Edge =>
        if(that._3 == this._3 && that._2 == this._2 && that._1 == this._1)
          true
        else false
      case _ =>
        false
    }
  }
  /**
    * A bracket () enclosed form of the directed edge
    */
  override def toString = s"""(${_1}, ${_2}, ${_3})"""
}
/**
  * Edge object methods
  */
object Edge {
  /**
    * Simplify construction
    */
  def apply(_1 : Tag, _2 : Tag, _3 : Tag) : Edge = new Edge(_1, _2, _3)

}
/**
  * A graph is a collection of edges so could be a List[Edge]
  * but might also be a remote store.
  *  EdgeCollection is a simple trait for building a graph without other requirements
  */
trait EdgeCollection {

  def add(edge : Edge)

}
/**
  * Add the ability to delete an edge
  */
trait EditableEdgeCollection extends EdgeCollection {

  def delete(edge : Edge)

}
/**
  * An edge at its core is a triple of Tags
  * - a tag is uniquely identified by an URL like mechanism,
  * the only change from URL is that localName part is unrestricted allowing
  * for an in Tag serialization of objects
  * e.g a geographic point could be represented by
  * http://maps.xythings.com/(53.413,0.123)
  * where the localName (53.413,0.123) can be simply parsed to the scala Tuple of
  * type [Double, Double]
  */
trait Tag {
  val logger = java.util.logging.Logger.getLogger("semantic.Tag")

  val namespace : String
  val localName : String

  lazy val url = namespace + localName
  /**
    * hashCode and equals rely on the url value. Tags with matching urls are equal
    */
  override def hashCode = url.hashCode
  /**
    * hashCode and equals rely on url. Tags with matching urls are equal.
    */
  override def equals(any : Any) : Boolean = {
    any match {
      case that : Tag =>
        if(that.url == this.url)
          true
        else false
      case _ =>
        false
    }
  }
  /**
    * Make the tag easily readable
    */
  override def toString = "(" + namespace + "[" + localName + "])"
}
/**
  * Allow simple creation of Tags
  *
  */
object Tag {
  /**
    * Create a SimpleTag from a namespace and localName
    *
    * - we could force namespace to terminate in a special character # or / (as used in apply(url) but we leave this
    * to user discretion instead
    *
    */
  def apply(namespace : String, localName : String): Tag = new SimpleTag(namespace, localName)
  /**
    * Create a SimpleTag from an URL
    *  the mechanism assumes urls which contain a # are of the form http://......#foobar
    */
  def apply(url : String): Tag = {

    val pathIndex = max(url.lastIndexOf('#'), url.lastIndexOf('/'))

    val namespace = url.substring(0, pathIndex + 1)

    val localName = java.net.URLDecoder.decode(url.drop(namespace.length), "UTF-8")

    new SimpleTag(namespace, localName)
  }
}
/**
  * The most basic tag
  */
sealed class SimpleTag(val namespace : String, val localName : String) extends Tag