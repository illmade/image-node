package com.xythings.semantic

import org.specs2.mutable.Specification

/**
  * Created by tim on 15/05/2017.
  */
class IndexedGraphSpec extends Specification {
  "This is a specification for IndexedGraph".txt

  val pathTag = Tag("http://xythings.com/test/world")
  val wildResourceEdge = Edge(pathTag, Wildcard, Wildcard)
  val equalTag = Tag("http://xythings.com/test/", "world")

  val predicateIs = Tag("http://xythings.com/test/is")
  val wildPropertyEdge = Edge(Wildcard, predicateIs, Wildcard)
  val predicateIsNot = Tag("http://xythings.com/test/is_not")

  val wildResourcePropertyEdge = Edge(pathTag, predicateIs, Wildcard)

  val objectComplete = Tag("http://xythings.com/test/complete")
  val objectNotComplete = Tag("http://xythings.com/test/not_complete")

  val completeEdge = Edge(pathTag, predicateIs, objectComplete)
  val completeCompareEdge = Edge(pathTag, predicateIs, objectComplete)

  val wildPropertyObjectEdge = Edge(Wildcard, predicateIs, objectComplete)
  val wildObjectEdge = Edge(Wildcard, Wildcard, objectComplete)

  val incompleteEdge = Edge(pathTag, predicateIsNot, objectComplete)

  val worldResourceIndex = ResourceIndex(pathTag)
  val worldPropertyIndex = PropertyIndex(predicateIs)
  val worldResourcePropertyIndex = ResourcePropertyIndex(pathTag, predicateIs)
  val worldPropertyObjectIndex = PropertyObjectIndex(predicateIs, objectComplete)
  val worldObjectIndex = ObjectIndex(objectComplete)

  val indexedGraph = IndexedGraph(List(completeEdge, incompleteEdge), List(ResourceIndex, PropertyIndex))

  val resourceQuery = indexedGraph.query(worldResourceIndex)
  val propertyQuery = indexedGraph.query(worldPropertyIndex)

  "indexedGraph should" >> {
    " be a SimpleIndexedGraph" >> {
      indexedGraph must haveClass[SimpleIndexedGraph]
    }
  }
  "resourceQuery should" >> {
    " be Some[Iterable] edge" >> {
      resourceQuery must beSome[Iterable[Edge]]
    }
    " have both input edges" >> {
        resourceQuery mustEqual Some(List(incompleteEdge, completeEdge))
    }
  }
  "propertyQuery should" >> {
    " be Some[Iterable] edge" >> {
      propertyQuery must beSome[Iterable[Edge]]
    }
    " have both input edges" >> {
      propertyQuery mustEqual Some(List(completeEdge))
    }
  }
}
