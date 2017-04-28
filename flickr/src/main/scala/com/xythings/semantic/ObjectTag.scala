package com.xythings.semantic

/**
  * Created by tim on 28/04/2017.
  */

/**
  * ObjectTags carry a local that can be of any type
  */
trait ObjectTag extends Tag {

  val local : Any
  /**
    * The default localName is the toString method of the local [Any]
    */
  lazy val localName = local.toString

}
/**
  * To reliably allow Tag to carry non-string type they need a scope defined by a Tag
  * An ObjectCaster is used to identify is such a scope Tag exists
  */
trait ObjectCaster {
  /**
    * Create a mapping for a given Tag
    */
  def typeMapping(typeTag : Tag) = TypeTag.renamespace(typeTag) -> typeTag
  /**
    * Get the casting Tag for a namespace if available
    */
  def getCast(namespace : String) : Option[Tag]
  /**
    * An objectCaster can cast in a number of ways depending on where we are serializing
    */
  def cast(tag : ObjectTag) : Option[Any]
  /**
    * This is for use only by toString methods to identify objectTypes in localName
    */
  def castStringIdentifier(namespace : String) : String

}
/**
  * Prefer ObjectTags to be created by the apply method
  */
sealed class CastObjectTag(val namespace : String, val local : Any, objectCaster : ObjectCaster) extends ObjectTag {
  /**
    * Make sure the local is defined by the objectCaster
    */
  require(objectCaster.getCast(namespace)!=None, "No cast for namespace: " + namespace)

  override def toString = "(" + namespace + "[" + objectCaster.cast(this).get + "]" + objectCaster.castStringIdentifier(namespace) + ")"

}
/**
  * Use a specified objectCaster (defaults to SimpleObjectCaster) to create objectTags
  */
object ObjectTag {

  def apply(namespace : String, local : Any, objectCaster : ObjectCaster = SimpleObjectCaster) = new CastObjectTag(namespace, local, objectCaster)

}
/**
  * Set up object cast mappings for our most common types
  */
object SimpleObjectCaster extends ObjectCaster {

  val INTEGER_MAPPING = typeMapping(libraries.XmlSchemaTags.INTEGER)
  val INT_MAPPING = typeMapping(libraries.XmlSchemaTags.INT)
  val DOUBLE_MAPPING = typeMapping(libraries.XmlSchemaTags.DOUBLE)
  val DECIMAL_MAPPING = typeMapping(libraries.XmlSchemaTags.DECIMAL)
  val FLOAT_MAPPING = typeMapping(libraries.XmlSchemaTags.FLOAT)
  val DATE_MAPPING = typeMapping(libraries.XmlSchemaTags.DATE)
  val POINT_R2_MAPPING = typeMapping(libraries.XyGeometryTags.POINT_R2)
  val POINTS_R2_MAPPING = typeMapping(libraries.XyGeometryTags.POINTS_R2)
  val SEQUENCE_POINTS_R2_MAPPING = typeMapping(libraries.XyGeometryTags.SEQUENCE_POINTS_R2)

  private var castMap : Map[String, Tag] = Map[String, Tag](
    INT_MAPPING,
    INTEGER_MAPPING,
    FLOAT_MAPPING,
    DOUBLE_MAPPING,
    DECIMAL_MAPPING,
    DATE_MAPPING,
    POINT_R2_MAPPING,
    POINTS_R2_MAPPING,
    SEQUENCE_POINTS_R2_MAPPING
  )

  /**
    * Get the casting Tag for a namespace if available
    */
  def getCast(namespace : String) : Option[Tag] =	castMap.get(namespace)

  def getCast(tag : Tag) : Option[Tag] =	castMap.get(tag.namespace)

  /**
    * This is for use only by toString methods to identify objectTypes in localName
    */
  def castStringIdentifier(namespace : String) : String = {
    namespace match {
      case INTEGER_MAPPING._1 =>
        "^^&xsd;integer"
      case INT_MAPPING._1 =>
        "^^&xsd;int"
      case FLOAT_MAPPING._1 =>
        "^^&xsd;float"
      case DOUBLE_MAPPING._1 =>
        "^^&xsd;double"
      case DECIMAL_MAPPING._1 =>
        "^^&xsd;decimal"
      case DATE_MAPPING._1 =>
        "^^&xsd;date"
      case POINT_R2_MAPPING._1 =>
        "^^&geometry;pointR2"
      case POINTS_R2_MAPPING._1 =>
        "^^&geometry;pointsR2"
      case SEQUENCE_POINTS_R2_MAPPING._1 =>
        "^^&geometry;sequencePointsR2"
      case _ =>
        ""
    }
  }

  private val doubleNumber = """(\d\d)""".r
  /**
    * Generally the toString method is good enough for our simple casting, it will always succeed.
    * Dates could cause trouble if we wanted to preserve values through file - database read/write cycles
    */
  def cast(tag : ObjectTag): Option[String] = {
    tag.local match {
      case calendar : java.util.Calendar =>
        //We might want to check the namespace to decide which fields to show
        val year = calendar.get(java.util.Calendar.YEAR)
        val month = (calendar.get(java.util.Calendar.MONTH) + 1).toString match {
          case doubleNumber(number : String) =>
            number
          case digit =>
            "0" + digit
        }
        val day = calendar.get(java.util.Calendar.DAY_OF_MONTH).toString match {
          case doubleNumber(number : String) =>
            number
          case digit =>
            "0" + digit
        }
        Some(s"${year}-${month}-${day}")
      case date : java.util.Date =>
        //We might want to check the namespace to decide which fields to show
        val calendar = java.util.Calendar.getInstance()
        calendar.setTime(date)
        val year = calendar.get(java.util.Calendar.YEAR)
        val month = (calendar.get(java.util.Calendar.MONTH) + 1).toString match {
          case doubleNumber(number : String) =>
            number
          case digit =>
            "0" + digit
        }
        val day = calendar.get(java.util.Calendar.DAY_OF_MONTH).toString match {
          case doubleNumber(number : String) =>
            number
          case digit =>
            "0" + digit
        }
        Some(s"${year}-${month}-${day}")
      case _ =>
        Some(tag.local.toString)
    }

  }
}