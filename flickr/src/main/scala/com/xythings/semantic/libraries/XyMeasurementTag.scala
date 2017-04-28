package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._
/**
  * Create tags in the 'http://data.xythings.com/namespace/measure/' namespace
  */
object XyMeasurementTag {
  val namespaceMapping = "http://data.xythings.com/namespace/measure/"->"xy-measure"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}

object XyMeasurementTags {

  val KILOMETER = XyMeasurementTag("kilometer")
  val COUNT = XyMeasurementTag("count")
  val UNIQUE_COUNT = XyMeasurementTag("uniqueCount")

}

