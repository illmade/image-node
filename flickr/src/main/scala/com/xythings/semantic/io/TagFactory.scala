package com.xythings.semantic.io

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._
/**
  * A trait for constructing tags from strings
  */
trait TagFactory {
  val logger = java.util.logging.Logger.getLogger("TagFactory")
  /**
    * A factory may fail to construct a Tag and return None
    */
  def makeTag(value : String) : Option[Tag]
}
/**
  * Create a TypeTag(libraries.XmlSchemaTags.STRING)
  */
object StringTagFactory extends TagFactory {
  /**
    * makeTag will always succeed
    */
  def makeTag(value : String) : Option[Tag] = Some(TypeTag(libraries.XmlSchemaTags.STRING, value))

}

object DoubleTagFactory extends TagFactory {

  def makeTag(value : String) : Option[Tag] = Some(TypeTag(libraries.XmlSchemaTags.DOUBLE, value.toDouble))

}

object FloatTagFactory extends TagFactory {

  def makeTag(value : String) : Option[Tag] = Some(TypeTag(libraries.XmlSchemaTags.FLOAT, value.toFloat))

}

object IntegerTagFactory extends TagFactory {

  def makeTag(value : String) : Option[Tag] = {
    try {
      val intValue = value.toInt
      Some(TypeTag(libraries.XmlSchemaTags.INTEGER, intValue))
    }
    catch {
      case e : NumberFormatException =>
        logger.warning(s"Could not create Integer from $value: $e")
        None
    }
  }

}

object PointTagFactory extends TagFactory {

  private val pointR2r = "\\(*(-*\\d+\\.\\d+E*-*\\d*)\\s*,\\s*(-*\\d+\\.\\d+E*-*\\d*)\\)*".r

  def makeTag(value : String) : Option[Tag] = {
    value match {
      case pointR2r(_1 : String, _2 : String) =>
        val pointR2 : (Double, Double) = (_1.toDouble, _2.toDouble)
        Some(TypeTag(libraries.XyGeometryTags.POINT_R2, pointR2))
      case _ =>
        logger.warning("Could not parse value to point: " + value)
        None
    }
  }

}

object StringSequenceTagFactory extends TagFactory {

  private val stringListR = "\\[\"\"\"(.*?)\"\"\"\\]".r

  def makeTag(value : String) : Option[Tag] = {
    var strings : List[String] = Nil
    for (string <- stringListR.findAllIn(value).matchData)
      strings = string.group(1) :: strings

    Some(TypeTag(libraries.XyTextTags.STRING_SEQUENCE, strings))
  }
}

object PointsTagFactory extends TagFactory {

  val pointPairR = "(-*\\d+\\.\\d+E*-*\\d*)\\s*,\\s*(-*\\d+\\.\\d+E*-*\\d*)".r

  def makeTag(value : String) : Option[Tag] = {
    val points = formPoints(value)
    val pointsTag = TypeTag(libraries.XyGeometryTags.POINTS_R2, points.toIterable)
    Some(pointsTag)
  }

  def formPoints(value : String) : Iterable[(Double, Double)] =
    pointPairR.findAllIn(value).matchData.map(
      pair => (pair.group(1).toDouble, pair.group(2).toDouble)).toList

}

object SequencePointsTagFactory extends TagFactory {

  def makeTag(value : String) : Option[Tag] = {
    val points = formPoints(value)
    val pointsTag = TypeTag(libraries.XyGeometryTags.SEQUENCE_POINTS_R2, points.toIterable)
    Some(pointsTag)
  }

  def formPoints(value : String) : Iterable[Iterable[(Double, Double)]] = {
    value.split(", List").map(list => {
      PointsTagFactory.pointPairR.findAllIn(list).matchData.map(
        pair => (pair.group(1).toDouble, pair.group(2).toDouble)).toList

    }).toList


  }

}
/**
  * Create tags from valid XML date
  * we could (should?) use dataFormat instead
  */
object XmlDateTagFactory extends TagFactory {
  val yearR = """(\d\d\d\d)""".r
  val dateR = """(\d\d\d\d)-(\d\d)-(\d\d)-*(.*)""".r
  val timezoneR = """(\d\d) (\d\d)""".r

  def makeTag(value : String) : Option[Tag] = {
    value match {
      case yearR(year) =>
        val calendar = java.util.Calendar.getInstance
        calendar.set(year.toInt, 0, 1)
        val dateTag = TypeTag(libraries.XmlSchemaTags.DATE, calendar)
        Some(dateTag)
      case dateR(year, month, day, timezone) =>
        val calendar = java.util.Calendar.getInstance
        calendar.set(year.toInt, month.toInt - 1, day.toInt)
        if (timezone.length > 0) timezone match {
          case timezoneR(hour, minute) =>
            calendar.setTimeZone(new java.util.SimpleTimeZone( (hour+minute).toInt, ""))
          case _ =>
        }
        val dateTag = TypeTag(libraries.XmlSchemaTags.DATE, calendar.getTime())
        Some(dateTag)
      case _ =>
        logger.warning("Could not parse xml date from: " + value)
        None
    }
  }
}
