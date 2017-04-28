package com.xythings.xml

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._
/**
  * Creates a simple TagReader with mappings for the datatypes in SimpleObjectCaster
  */
object TagReader {
  /**
    * Create a tagReader with the given factoryMappings or the defaults if None specified
    */
  def apply(factoryMappings : Option[Map[Tag, io.TagFactory]]) : TagReader = {

    val tagFactories = factoryMappings match {
      case None =>
        Map[Tag, io.TagFactory](
          libraries.RdfTags.XML_LITERAL -> io.StringTagFactory,
          libraries.XmlSchemaTags.STRING -> io.StringTagFactory,
          libraries.XmlSchemaTags.DOUBLE -> io.DoubleTagFactory,
          libraries.XmlSchemaTags.DECIMAL -> io.DoubleTagFactory,
          libraries.XmlSchemaTags.FLOAT -> io.FloatTagFactory,
          libraries.XmlSchemaTags.INTEGER -> io.IntegerTagFactory,
          libraries.XmlSchemaTags.INT -> io.IntegerTagFactory,
          libraries.W3TimeTags.YEAR -> io.XmlDateTagFactory,
          libraries.XmlSchemaTags.DATE -> io.XmlDateTagFactory,
          libraries.XyGeometryTags.POINT_R2 -> io.PointTagFactory,
          libraries.XyGeometryTags.POINTS_R2 -> io.PointsTagFactory,
          libraries.XyGeometryTags.SEQUENCE_POINTS_R2 -> io.SequencePointsTagFactory
        )
      case _ =>
        factoryMappings.get
    }
    new SimpleTagReader(tagFactories)
  }
}
/**
  * A trait for reading tags
  */
trait TagReader {

  def readTag(value : String, datatype : Option[Tag]) : Option[Tag]

}
/**
  * Register deserializers for objects with datatype in rdf files
  */
sealed class SimpleTagReader(val tagFactories : Map[Tag, io.TagFactory]) extends TagReader {

  private val logger = java.util.logging.Logger.getLogger("TagReader")
  val hatCheckR = "\"?(.*?)\"?\\^\\^(http://.*)".r

  def readTag(value : String, datatype : Option[Tag] = None) : Option[Tag] = {
    datatype match {
      case None =>
        value.trim match {
          case hatCheckR(value : String, dataTypeUrl) =>
            readTag(value, new Some(Tag(dataTypeUrl)))
          case _ =>
            Some(TypeTag(libraries.XmlSchemaTags.STRING, value))
        }
      case some =>
        tagFactories.get(some.get) match {
          case None =>
            logger.warning("No factory for type: " + datatype)
            Some(TypeTag(libraries.XmlSchemaTags.STRING, value))
          case some =>
            some.get.makeTag(value)
        }
    }
  }
}