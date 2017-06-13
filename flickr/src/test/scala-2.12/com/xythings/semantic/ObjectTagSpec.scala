package com.xythings.semantic

import java.util.Calendar

import org.specs2.mutable.Specification

/**
  * Created by tim on 15/05/2017.
  */
class ObjectTagSpec extends Specification {

  println(SimpleObjectCaster.INT_MAPPING)

  val intNamespace = "http://www.w3.org/2001/XMLSchema/int/"

  val simpleObjectTag = ObjectTag(intNamespace, 2) //namespace : String, local : Any,

  val calendar = java.util.Calendar.getInstance()

  calendar.set(2017, Calendar.DECEMBER, 27)

  val dateNamespace = "http://www.w3.org/2001/XMLSchema/date/"

  val dateObjectTag = ObjectTag(dateNamespace, calendar)


  """SimpleObject should """ >> {
    " have an int namespace" >> {
      simpleObjectTag.namespace mustEqual intNamespace
    }
    " have '2' as localName" >> {
      simpleObjectTag.localName mustEqual "2"
    }
    " have 'http://www.w3.org/2001/XMLSchema/int/2' as url" >> {
      simpleObjectTag.url mustEqual "http://www.w3.org/2001/XMLSchema/int/2"
    }
    " have an object local of 2" >> {
      simpleObjectTag.local mustEqual 2
    }
  }

  """dateObjectTag should """ >> {
    " have a date namespace" >> {
      dateObjectTag.namespace mustEqual dateNamespace
    }
    " have '2' as localName" >> {
      dateObjectTag.toString mustEqual "(http://www.w3.org/2001/XMLSchema/date/[2017-12-27]^^&xsd;date)"
    }
  }

}
