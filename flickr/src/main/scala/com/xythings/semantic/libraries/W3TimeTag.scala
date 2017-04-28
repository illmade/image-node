package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic.Tag

object W3TimeTag {
  val namespaceMapping = ("http://www.w3.org/2006/time#", "time")
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}

object W3TimeTags {
  val YEAR = W3TimeTag("year")
  val MONTH = W3TimeTag("month")
  val DAY = W3TimeTag("day")
  val HOUR = W3TimeTag("hour")
  val MINUTE = W3TimeTag("minute")
}

