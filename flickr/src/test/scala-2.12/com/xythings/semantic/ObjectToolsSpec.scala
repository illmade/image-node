package com.xythings.semantic

import org.specs2.mutable.Specification

/**
  * Created by tim on 30/05/2017.
  */
class ObjectToolsSpec extends Specification {
  "This is a specification for ObjectTools".txt

  val twodTag = ObjectTools.getTag(2.0)
  val twodVal = ObjectTools.getDouble(twodTag)

  val twofTag = ObjectTools.getTag(2.0f)
  val twofVal = ObjectTools.getFloat(twofTag)

  val twoTag = ObjectTools.getTag(2)
  val twoVal = ObjectTools.getInt(twoTag)

  val twoTwoTag = ObjectTools.getTag((2.0, 2.0))
  val twoTwoVal = ObjectTools.getPoint(twoTwoTag)

  "twodTag must" >> {
    " be an instanceOf ObjectTag" >> {
      twodTag must beAnInstanceOf[ObjectTag]
    }
  }
  "twodVal must" >> {
    " have get = 2.0 " >> {
      twodVal.get mustEqual 2.0
    }
  }

  "twofTag must" >> {
    " be an instanceOf ObjectTag" >> {
      twofTag must beAnInstanceOf[ObjectTag]
    }
  }
  "twofVal must" >> {
    " have get = 2f " >> {
      twofVal.get mustEqual 2f
    }
  }

  "twoTag must" >> {
    " be an instanceOf ObjectTag" >> {
      twoTag must beAnInstanceOf[ObjectTag]
    }
  }
  "twoVal must" >> {
    " have get = 2 " >> {
      twoVal.get mustEqual 2
    }
  }

  "twoTwoTag must" >> {
    "have an instanceOf ObjectTag" >> {
      twoTwoTag must beAnInstanceOf[ObjectTag]
    }
  }
  "twoTwoVal must" >> {
    "have a get of (2.0, 2.0)" >> {
      twoTwoVal.get mustEqual (2.0, 2.0)
    }
  }
}
