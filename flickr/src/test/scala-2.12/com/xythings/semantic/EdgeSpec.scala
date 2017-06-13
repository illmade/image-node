package com.xythings.semantic

import org.specs2.mutable.Specification

/**
  * Created by tim on 15/05/2017.
  */
class EdgeSpec extends Specification {
  "This is a specification for Edge".txt

  val simplestTag = Tag("hello/", "world")

  val pathTag = Tag("http://xythings.com/test/world")
  val equalTag = Tag("http://xythings.com/test/", "world")

  val predicateIs = Tag("http://xythings.com/test/is")
  val predicateIsNot = Tag("http://xythings.com/test/is_not")

  val objectComplete = Tag("http://xythings.com/test/complete")
  val objectNotComplete = Tag("http://xythings.com/test/not_complete")

  val completeEdge = Edge(simplestTag, predicateIs, objectComplete)
  val completeCompareEdge = Edge(simplestTag, predicateIs, objectComplete)

  val incompleteEdge = Edge(simplestTag, predicateIs, objectNotComplete)

  "simplestTag should" >> {
    " have hello as its namespace" >> {
      simplestTag.namespace mustEqual "hello/"
    }
    " have world as localName" >> {
      simplestTag.localName mustEqual "world"
    }
    " have 'hello/world' as url" >> {
      simplestTag.url mustEqual "hello/world"
    }
  }

  "pathTag should" >> {
    " have http://xythings.com/test/ as its namespace" >> {
      pathTag.namespace mustEqual "http://xythings.com/test/"
    }
    " have world as localName" >> {
      pathTag.namespace mustEqual "http://xythings.com/test/"
    }
  }

  "equalTag should" >> {
    " equal pathTag" >> {
      pathTag mustEqual equalTag
    }
    " not equal simplestTag" >> {
      pathTag mustNotEqual simplestTag
    }
  }

  "completeEdge should" >> {
    " have a predicate tag predicateIs" >> {
      completeEdge._2 mustEqual completeEdge.property
    }
    " have a predicate tag predicateIs" >> {
      completeEdge._2 mustEqual predicateIs
    }
    " be equal to completeCompareEdge" >> {
      completeEdge mustEqual completeCompareEdge
    }
    " be not equal to completeCompareEdge" >> {
      completeEdge mustNotEqual incompleteEdge
    }
  }

}
