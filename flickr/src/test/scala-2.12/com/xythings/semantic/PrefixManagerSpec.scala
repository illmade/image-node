package com.xythings.semantic

import org.specs2.mutable.Specification

/**
  * Created by tim on 30/05/2017.
  */
class PrefixManagerSpec extends Specification {
  "Test Spec for PrefixManager".txt

  val prefixMap = Map(("http://www.test.com/apples/" -> "apples"), ("http://www.test.com/oranges/" -> "oranges"))

  val cooking = Tag("http://www.test.com/apples/cooking")

  val prefixManager = PrefixManager(Some(prefixMap))

  "PrefixManager should" >> {
    "give prefix 'apples' for Tag 'http://www.test.com/apples/cooking'" >> {
      prefixManager.prefixFor(cooking) must beSome("apples")
    }
    "give namespace 'http://www.test.com/apples/' for 'apples'" >> {
      prefixManager.namespaceFor("apples") must beSome("http://www.test.com/apples/")
    }
  }

}
