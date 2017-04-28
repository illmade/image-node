package com.xythings.semantic

/**
  * Created by tim on 28/04/2017.
  *
  * Type tags are defined by their parent paths.
  */
object TypeTag {
  /**
    * The typed tags extend their type namespace
    * e.g.
    * A test type http://data.xythings.com/type/test
    * can become a root of test tags
    * so test1 would have the namespace
    * http://data.xythings.com/type/test/
    * and url
    * http://data.xythings.com/type/test/test1
    *
    * namespaces ending with # are changed to a /
    *
    * in particular in XML types
    * http://www.w3.org/2001/XMLSchema#[integer] will renamepace to
    * http://www.w3.org/2001/XMLSchema/integer/
    *
    */
  def renamespace(typeSource : Tag) = typeSource.url.replace('#', '/') + "/"
  /**
    * Create typed Tags using by default the SimpleObjectCaster
    */
  def apply(typeSource : Tag, value : Any, objectCaster : ObjectCaster = SimpleObjectCaster) : ObjectTag = ObjectTag(renamespace(typeSource), value, objectCaster)
  /**
    * Create typed Tags using a simple localName
    */
  def apply(typeSource : Tag, localName : String) : Tag = Tag(renamespace(typeSource), localName)
}

