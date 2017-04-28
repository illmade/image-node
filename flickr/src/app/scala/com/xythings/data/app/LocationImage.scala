package com.xythings.data.app

/**
  * Created by tim on 28/04/2017.
  */
import com.xythings.semantic._
import com.xythings.data.flickr._
import com.xythings.geometry._
import java.io.FileInputStream
import java.util.Properties


object FlickrPlace extends App {

  val logger = java.util.logging.Logger.getLogger("MakeFlickrPlace")

  val prop = new Properties();

  try {

    val resource = getClass().getClassLoader().getResource("config.prop")

    val input = new FileInputStream(resource.getFile())

    prop.load(input)

  }

  val key = new LocationApiKey(prop.getProperty("key", "empty"))

  val flickrRest = new FlickrRest(key)

  val photoReader = new PhotoReader(key)

  val path = args(0)
  val placeName = args(1)
  val boundsString = args(2).trim
  val maxImages = 2000

  private val boundsR = "\\(\\((-*\\d+\\.\\d+E*-*\\d*),\\s*(-*\\d+\\.\\d+E*-*\\d*)\\),\\s*\\((-*\\d+\\.\\d+E*-*\\d*),\\s*(-*\\d+\\.\\d+E*-*\\d*)\\)\\)".r

  boundsString match {
    case boundsR(latA : String, lngA : String, latB : String, lngB : String) =>
      val bounds = ((latA.toDouble, lngA.toDouble),(latB.toDouble, lngB.toDouble))
      logger.info(s"Bounds is: $bounds")
      mk(bounds)
  }

  def mk(bounds : ((Double, Double), (Double, Double))){
    val labelName = placeName.replace(' ', '_')

    val place = Tag("http://data.xythings.com/namespace/flickr/ie/", labelName)
    val outputFile = path + labelName.toLowerCase + ".rdf"

    val placesUrl = flickrRest.findPhotos(bounds = bounds, place = Some(placeName))

    val xmlReader = new com.xythings.xml.HttpXmlReader

    val response = xmlReader.getXml(placesUrl)

    var points : List[(Double, Double)] = Nil
    var uids : List[String] = Nil

    var placemarkMap : Map[String, PhotoDetail] = Map.empty

    var userMap : Map[String, Int] = Map.empty

    def userIncrement(detail : PhotoDetail) : Int = {
      userMap.get(detail.owner) match {
        case None =>
          userMap += detail.owner -> 1
          1
        case some =>
          val newCount = some.get +1
          userMap += detail.owner -> newCount
          newCount
      }
    }

    var detailGraph : List[Edge] = Nil

    def graphDetail(detail : PhotoDetail){

      val about = Tag(detail.url)
      val ownerTag = TypeTag(libraries.XmlSchemaTags.STRING, detail.owner)
      detailGraph = Edge(about, libraries.DcmiTags.CREATOR, ownerTag) :: detailGraph
      val downloadTag = TypeTag(libraries.XmlSchemaTags.INT, detail.canDownload)
      detailGraph = Edge(about, libraries.DcmiTags.RIGHTS, downloadTag) :: detailGraph
      val secretTag = Tag(detail.secret)
      detailGraph = Edge(about, libraries.DcmiTags.IMAGE, secretTag) :: detailGraph
      detailGraph = Edge(about, libraries.SkosTags.RELATED, TypeTag(libraries.XmlSchemaTags.STRING, detail.tags.toString)) :: detailGraph
      val point = TypeTag(libraries.XyGeometryTags.POINT_R2, detail.latitudeLongitude)
      detailGraph = Edge(about, libraries.WgsTags.LAT_LONG, point) :: detailGraph

    }

    response match {
      case None =>
        logger.warning("Response does not contain XML " )
      case xmlResponse =>
        if((xmlResponse.get \ "@stat").text == "ok"){

          val initialResponse = xmlResponse.get \ "photos"
          val numberOfPages = (initialResponse \ "@pages").text.toInt
          val imagesPerPage = (initialResponse \ "@perpage").text.toInt

          logger.info(s"Got ${numberOfPages} page(s) for ${placeName} at ${imagesPerPage} images per page")

          addPage(xmlResponse.get, 1)

          for(page <- 2 to numberOfPages if points.length < maxImages){
            logger.info("moved to page " + page + " of " + numberOfPages)

            val pageUrl = flickrRest.findPhotos(bounds = bounds, place = Some(placeName), page = page)
            val newResponse = xmlReader.getXml(pageUrl)
            newResponse match {
              case None =>
                logger.warning("response does not contain XML")
              case newResponse =>
                if((newResponse.get \ "@stat").text == "ok")
                  addPage(newResponse.get, page)
            }

          }

          logger.info("Upper bound of imageCount: " + (numberOfPages * imagesPerPage))

          def addPage(page : scala.xml.Elem, pageNumber : Int){
            val photos = page \ "photos"

            for(photo <- photos \ "photo"){
              val photoId = (photo \ "@id").text

              photoReader.getDetail(photoId) match {
                case None =>
                case details =>
                  val detail = details.get
                  //Don't let a single user skew things too much
                  if(userIncrement(detail) < 15){
                    if(!points.contains(detail.latitudeLongitude)){
                      points = detail.latitudeLongitude :: points
                      graphDetail(detail)
                      placemarkMap += photoId -> details.get
                    }
                    uids = detail.owner :: uids
                  }
                  else
                    logger.info(s"Photo ignored, user ${detail.owner} has too many entries: ${userMap.get(detail.owner)}")
              }
            }
          }
        }

        val imageSample = points.distinct
        val hull = com.xythings.geometry.MonotoneChain(imageSample)

        var edges : List[Edge] = Nil

        val labelTag = TypeTag(libraries.XmlSchemaTags.STRING, placeName)
        edges = Edge(place, libraries.RdfsTags.LABEL, labelTag) :: edges

        val pointsTag = TypeTag(libraries.XyGeometryTags.POINTS_R2, imageSample)
        edges = Edge(place, libraries.XyGeometryTags.POINTS_R2, pointsTag) :: edges

        val hullTag = TypeTag(libraries.XyGeometryTags.POINTS_R2, hull)
        edges = Edge(place, libraries.XyGeometryTags.POLYGON, hullTag) :: edges

        val countTag = TypeTag(libraries.XmlSchemaTags.INT, imageSample.length)
        edges = Edge(place, libraries.XyMeasurementTags.COUNT, countTag) :: edges

        val uniqueTag = TypeTag(libraries.XmlSchemaTags.INT, uids.distinct.length)
        edges = Edge(place, libraries.XyMeasurementTags.UNIQUE_COUNT, uniqueTag) :: edges

        logger.info("Outputting: " + outputFile)
        com.xythings.xml.RdfWriter.save(outputFile, edges)

        //now index
        val zoom = 14
        val indexFile = path + labelName.toLowerCase + s"_${zoom}_geoindex.rdf"

        val placeGraph = IndexedGraph(edges)

        val indexEdges = PlaceIndex.indexPlace(placeGraph, zoom)

        if(!indexEdges.isEmpty){
          com.xythings.xml.RdfWriter.save(indexFile, indexEdges.toList.distinct)
          logger.info("Output index: " + indexFile)
        }
        else
          logger.warning("No indexes created for " + place)

        val detailFile = path + labelName.toLowerCase + s"_details.rdf"

        if(!detailGraph.isEmpty){
          com.xythings.xml.RdfWriter.save(detailFile, detailGraph)
          logger.info("Output info: " + detailFile)
        }
        else
          logger.warning("No info created for " + place)

    }
  }

}

object PlaceIndex {

  val logger = java.util.logging.Logger.getLogger("PlaceIndex")

  def indexPlace(placeGraph : IndexedGraph, zoom : Int) : Iterable[Edge] = {

    val projection = new raster.ZoomProjection(zoom)
    val indexer = new index.ZoomIndex(projection)

    var indexEdges : Iterable[Edge] = Nil

    placeGraph.query(PropertyIndex(libraries.XyGeometryTags.POINTS_R2)) match {
      case None =>
        logger.warning("PlaceGraph does not contain points data: " + placeGraph)
      case some =>
        for(pointsEdge <- some.get){
          ObjectTools.getPoints(pointsEdge._3) match {
            case None =>
              logger.warning("No points for: " + pointsEdge)
            case points =>
              val shape = MonotoneChain(points.get)
              if (!shape.isEmpty) {
                indexEdges = indexer.indexShape(shape.toList).foldLeft(List[Edge]()){
                  (list, point) => Edge(index.ZoomIndex.indexTag(point, zoom), libraries.XyGeoTags.GRID_REFERENCE_FOR, pointsEdge.identifier) :: list
                } ++ indexEdges
              }
          }
        }
    }
    indexEdges
  }

}
