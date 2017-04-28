package com.xythings.semantic.libraries

import com.xythings.semantic.Tag

/**
  * Created by tim on 28/04/2017.
  * Create tags in the 'http://data.xythings.com/namespace/geo/' namespace
  */
object XyGeoTag {
  val namespaceMapping = "http://data.xythings.com/namespace/geo/"->"xy-geo"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}

object XyGeoTags {

  val GRID_REFERENCE = XyGeoTag("gridReference")
  val GRID_REFERENCE_FOR = XyGeoTag("gridReferenceFor")

}
