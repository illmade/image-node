package com.xythings.semantic.libraries

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._

object SkosTag {
  val namespaceMapping = "http://www.w3.org/2004/02/skos/core#" -> "skos"
  def apply(localName : String) = Tag(namespaceMapping._1, localName)
}
/**
  * A library of tags in the "http://www.w3.org/2002/07/owl#" namespace
  */
object SkosTags {

  val BROADER = SkosTag("broader")
  val NARROWER = SkosTag("narrower")
  val HAS_TOP_CONCEPT = SkosTag("hasTopConcept")
  val TOP_CONCEPT_OF = SkosTag("topConceptOf")
  val TOP_CONCEPT = SkosTag("topConcept")

  val MAPPING_RELATION = SkosTag("mappingRelation")
  val EXACT_MATCH = SkosTag("exactMatch")
  val BROAD_MATCH = SkosTag("broadMatch")
  val RELATED_MATCH = SkosTag("relatedMatch")
  val CLOSE_MATCH = SkosTag("closeMatch")

  val IN_SCHEME = SkosTag("inScheme")
  val CONCEPT_SCHEME = SkosTag("ConceptScheme")

  val RELATED = SkosTag("related")
  val BROADER_TRANSITIVE = SkosTag("broaderTransitive")
  val NARROWER_TRANSITIVE = SkosTag("narrowerTransitive")

  val EXAMPLE = SkosTag("example")
  val CHANGE_NOTE = SkosTag("changeNote")

  val COLLECTION = SkosTag("Collection")
  val MEMBER = SkosTag("member")

  val PREF_LABEL = SkosTag("prefLabel")
  val ALT_LABEL = SkosTag("altLabel")
  val HIDDEN_LABEL = SkosTag("hiddenLabel")

}
