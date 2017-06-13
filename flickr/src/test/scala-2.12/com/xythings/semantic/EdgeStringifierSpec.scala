package com.xythings.semantic

import org.specs2.mutable.Specification

/**
  * Created by tim on 15/05/2017.
  */
class EdgeStringifierSpec extends Specification {
  "This is a specification for EdgeStringifier".txt

  val simplestStringifier = EdgeStringifier(None)

  val pathTag = Tag("http://xythings.com/test/world")

  val predicateIs = Tag("http://xythings.com/test/is")
  val predicateIsNot = Tag("http://xythings.com/test/is_not")

  val objectComplete = Tag("http://xythings.com/test/complete")
  val objectNotComplete = Tag("http://xythings.com/test/not_complete")

  val completeEdge = Edge(pathTag, predicateIs, objectComplete)
  val completeCompareEdge = Edge(pathTag, predicateIs, objectComplete)

  val incompleteEdge = Edge(pathTag, predicateIs, objectNotComplete)

  val stringEdge = simplestStringifier.stringify(incompleteEdge)

  "EdgeStringifier should" >> {
    " create a string from completeEdge" >> {
      stringEdge must haveClass[String]
    }
    " create a string with 3 http components" >> {
      stringEdge mustEqual "(http://xythings.com/test/world, http://xythings.com/test/is, http://xythings.com/test/not_complete)"
    }
  }
}
