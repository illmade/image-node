package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic.Tag

object WgsTag {
  val namespaceMapping = ("http://www.w3.org/2003/01/geo/wgs84_pos#", "wgs")
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}

object WgsTags {
  val LOCATION = WgsTag("location")
  val LATITUDE = WgsTag("lat")
  val LONGITUDE = WgsTag("long")
  val LAT_LONG = WgsTag("lat_long")
  val GEO_POINT = WgsTag("Point")
}

