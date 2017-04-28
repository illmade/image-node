package com.xythings.xml

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._
/**
  * Serialize a tag to RdfTriple form
  */
trait TagWriter {

  def writeTag(tag : Tag) : (Option[String], String)

}
/**
  * Some general methods for TagWriter
  */
object TagWriter {

  def safeName(localName : String) : String = {
    val andVersion = localName.replaceAll("&", "&amp;")
    val aposVersion = andVersion.replaceAll("'", "&apos;")
    val quotVersion = aposVersion.replaceAll("\"", "&quot;")
    val ltVersion = quotVersion.replaceAll("<", "&lt;")
    val safeVersion = ltVersion.replaceAll(">", "&gt;")
    safeVersion
  }

}

class SimpleTagWriter(objectCaster : ObjectCaster = SimpleObjectCaster,
                      prefixManager : PrefixManager = impl.PrefixManagerImpl(None)) extends TagWriter {

  private val logger = java.util.logging.Logger.getLogger("SimpleTagWriter")

  def writeTag(tag : Tag) : (Option[String], String) = {
    var dataType : Option[String] = None
    var localName : String = tag match {
      case objectTag : ObjectTag =>
        objectCaster.getCast(objectTag.namespace) match {
          case None =>
            //Try to avoid this - only the localName is serialized
            logger.warning("No datatype for: " + objectTag)
            val url = objectTag.namespace + TagWriter.safeName(objectTag.localName)
            url
          case some =>
            prefixManager.prefixFor(some.get) match {
              case None =>
                logger.warning("No prefix for: " + some.get)
              case entity =>
                dataType = Some("&" + entity.get + ";" + some.get.localName)
            }
            val objectWriter = new StringBuilder
            //At the moment we only need to serialize STRING_SEQUENCE with a special case
            some.get match {
              case libraries.XyTextTags.STRING_SEQUENCE =>
                objectTag.local match {
                  case strings : Iterable[Any] =>
                    objectWriter.append("List(")
                    objectWriter.append("[\"\"\"" + strings.head.toString + "\"\"\"]")
                    for(string <- strings.tail)
                      objectWriter.append(", [\"\"\"" + string + "\"\"\"]")
                    objectWriter.append(")")
                }
              case _ =>
                val cast = objectCaster.cast(objectTag) match {
                  case None =>
                    objectTag.localName
                  case some =>
                    some.get
                }
                objectWriter.append(TagWriter.safeName(cast.toString))
            }
            objectWriter.toString
        }
      case _ =>
        TagWriter.safeName(tag.localName)
    }
    (dataType, localName)
  }
}
