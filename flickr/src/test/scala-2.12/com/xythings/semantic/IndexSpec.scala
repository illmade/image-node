package com.xythings.semantic

import org.specs2.mutable.Specification

/**
  * Created by tim on 15/05/2017.
  */
class IndexSpec extends Specification {
  "This is a specification for Index".txt

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

  val incompleteEdge = Edge(pathTag, predicateIs, objectNotComplete)

  val worldResourceIndex = ResourceIndex(pathTag)
  val worldPropertyIndex = PropertyIndex(predicateIs)
  val worldResourcePropertyIndex = ResourcePropertyIndex(pathTag, predicateIs)
  val worldPropertyObjectIndex = PropertyObjectIndex(predicateIs, objectComplete)
  val worldObjectIndex = ObjectIndex(objectComplete)

  "worldResourceIndex should" >> {
    " be an Edge" >> {
      worldResourceIndex must haveClass[Edge]
    }
    " equal wildResourceEdge" >> {
      worldResourceIndex mustEqual wildResourceEdge
    }
  }
  "worldResourceIndex should" >> {
    " be an Edge" >> {
      worldPropertyIndex must haveClass[Edge]
    }
    " equal wildResourceEdge" >> {
      worldPropertyIndex mustEqual wildPropertyEdge
    }
  }
  "worldResourcePropertyIndex should" >> {
    " be an Edge" >> {
      worldResourcePropertyIndex must haveClass[Edge]
    }
    " equal wildResourcePropertyEdge" >> {
      worldResourcePropertyIndex mustEqual wildResourcePropertyEdge
    }
  }
  "worldPropertyObjectIndex should" >> {
    " be an Edge" >> {
      worldPropertyObjectIndex must haveClass[Edge]
    }
    " equal wildPropertyObjectEdge" >> {
      worldPropertyObjectIndex mustEqual wildPropertyObjectEdge
    }
  }
  "worldObjectIndex should" >> {
    " be an Edge" >> {
      worldObjectIndex must haveClass[Edge]
    }
    " equal wildPropertyObjectEdge" >> {
      worldObjectIndex mustEqual wildObjectEdge
    }
  }
}
