package com.xythings.semantic

/**
  * Created by tim on 28/04/2017.
  *
  * Build representations of edges as strings for Map indexes, Database ids etc..
  */
trait EdgeStringifier {
  /**
    * Return the simplified string representation
    */
  def stringify(edge : Edge) : String

}
/**
  * Create a simple EdgeStringifier
  */
object EdgeStringifier {

  def apply(prefixManager : Option[PrefixManager] = None) = {
    prefixManager match {
      case None =>
        new SmallEdgeStringifier(PrefixManager(None))
      case some =>
        new SmallEdgeStringifier(some.get)
    }
  }

}
/**
  * A prefix manager enables us to build simplified representations of edges as strings
  */
class SmallEdgeStringifier(prefixManager : PrefixManager) extends EdgeStringifier {
  /**
    * Form a simplified representation of an edge
    */
  def stringify(edge : Edge) : String = {
    var indexBuilder : StringBuilder = new StringBuilder
    indexBuilder = indexBuilder.append("(")
    def build(tag : Tag){
      tag match {
        case Wildcard =>
          indexBuilder = indexBuilder.append("(_)")
        case _ =>
          prefixManager.prefixFor(tag) match {
            case None =>
              indexBuilder = indexBuilder.append(tag.url)
            case some =>
              indexBuilder = indexBuilder.append("(").append(some.get).append(":").append(tag.localName).append(")")
          }
      }
    }
    build(edge._1)
    indexBuilder = indexBuilder.append(", ")
    build(edge._2)
    indexBuilder = indexBuilder.append(", ")
    build(edge._3)
    indexBuilder.append(")").result
  }
}
