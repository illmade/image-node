package com.xythings.data.flickr

import com.google.api.client.auth.oauth2.{AuthorizationCodeFlow, BearerToken, ClientParametersAuthentication, Credential}
import com.google.api.client.auth.oauth._
import com.google.api.client.extensions.java6.auth.oauth2.AuthorizationCodeInstalledApp
import com.google.api.client.http._
import com.google.api.client.http.javanet.NetHttpTransport
import com.google.api.client.json.JsonObjectParser
import com.google.api.client.json.jackson2.JacksonFactory
import com.google.api.client.util.store
import com.google.api.client.util.store.FileDataStoreFactory
import com.xythings.data.flickr.impl.{LocalAuthorizer, LocalServiceReceiver}

/**
  * Created by tim on 27/04/2017.
  */
class FlickrUrl(val encodedUrl: String) extends GenericUrl(encodedUrl){

  var fields: String = ""

  def getFields = fields

  def setFields(fields: String): Unit ={
    this.fields = fields
  }

}

class OAuthReader(httpRequestFactory : HttpRequestFactory) extends com.xythings.xml.RemoteXmlReader {

  def getXml(lookup : String) : Option[scala.xml.Elem] = {
    val flickrUrl = new FlickrUrl(lookup)
    val request = httpRequestFactory.buildGetRequest(flickrUrl)
    val simpleResponse = request.execute.parseAsString
    if(simpleResponse.startsWith("<?xml"))
      Some(scala.xml.XML.loadString(simpleResponse))
    else
      None
  }
}

object Authorize {

  /** Directory to store user credentials. */
  val DATA_STORE_DIR =
    new java.io.File(System.getProperty("user.home"), ".store/flick")

  /** OAuth 2 scope. */
  val SCOPE = "read"

  /** Port in the "Callback URL". */
  val PORT = 8080

  /** Domain name in the "Callback URL". */
  val DOMAIN = "127.0.0.1"

  val DATA_STORE_FACTORY = new FileDataStoreFactory(DATA_STORE_DIR)

  /** Global instance of the HTTP transport. */
  val HTTP_TRANSPORT = new NetHttpTransport

  /** Global instance of the JSON factory. */
  val JSON_FACTORY = new JacksonFactory

  val TOKEN_SERVER_URL = "https://www.flickr.com/services/oauth/request_token"
  val AUTHORIZATION_SERVER_URL = "https://www.flickr.com/services/oauth/authorize"
  val ACCESS_TOKEN_URL = "https://www.flickr.com/services/oauth/access_token"

}

class OAuth1aAuthorize(credentials : OAuthClientCredentials) {

  import Authorize._

  val signer = new OAuthHmacSigner
  signer.clientSharedSecret = credentials.API_SECRET

  val createOAuthParameters : OAuthParameters = {
    val authorizer = new OAuthParameters
    authorizer.consumerKey = credentials.API_KEY
    authorizer.signer = signer
    authorizer
  }

  def authorize : HttpRequestFactory = {
    val receiver = new LocalServiceReceiver.Builder().setHost(DOMAIN).setPort(PORT).build
    val callback = receiver.getRedirectUri
    val localAuthorizer = new LocalAuthorizer(receiver)

    val temporaryToken = new OAuthGetTemporaryToken(TOKEN_SERVER_URL)

    temporaryToken.transport = HTTP_TRANSPORT
    temporaryToken.signer = signer
    temporaryToken.consumerKey = credentials.API_KEY
    temporaryToken.callback = callback

    val temporaryCredentialsResponse = temporaryToken.execute

    signer.tokenSharedSecret = temporaryCredentialsResponse.tokenSecret

    val temporaryTokenUrl = new OAuthAuthorizeTemporaryTokenUrl(AUTHORIZATION_SERVER_URL)
    temporaryTokenUrl.temporaryToken = temporaryCredentialsResponse.token

    val code = localAuthorizer.authorize(temporaryTokenUrl)

    val accessToken = new OAuthGetAccessToken(ACCESS_TOKEN_URL)
    accessToken.transport = HTTP_TRANSPORT
    accessToken.temporaryToken = temporaryCredentialsResponse.token
    accessToken.signer = signer
    accessToken.consumerKey = credentials.API_KEY
    accessToken.verifier = code

    val credentialsResponse = accessToken.execute

    println(credentialsResponse.token)
    println(credentialsResponse.tokenSecret)

    signer.tokenSharedSecret = credentialsResponse.tokenSecret
    val signingParameters = createOAuthParameters
    signingParameters.token = credentialsResponse.token

    Authorize.HTTP_TRANSPORT.createRequestFactory(new HttpRequestInitializer {
      def initialize(request : HttpRequest)  {
        createOAuthParameters.initialize(request)
        request.setParser(new JsonObjectParser(Authorize.JSON_FACTORY))
      }
    })
  }
}

class OAuth2Authorize(credentials : OAuthClientCredentials) {

  import Authorize._

  /** Authorizes the installed application to access user's protected data. */
  def authorize : Credential = {
    credentials.errorIfNotSpecified
    // set up authorization code flow
    val flow = new AuthorizationCodeFlow.Builder(BearerToken
      .authorizationHeaderAccessMethod(),
      HTTP_TRANSPORT,
      JSON_FACTORY,
      new GenericUrl(TOKEN_SERVER_URL),
      new ClientParametersAuthentication(
        credentials.API_KEY, credentials.API_SECRET),
      credentials.API_KEY,
      AUTHORIZATION_SERVER_URL).setScopes(java.util.Arrays.asList(SCOPE))
      .setDataStoreFactory(DATA_STORE_FACTORY)

    val builtFlow = flow.build
    // authorize
    val receiver = new impl.LocalServiceReceiver.Builder().setHost(DOMAIN).setPort(PORT).build
    new AuthorizationCodeInstalledApp(builtFlow, receiver).authorize("hawkins.tim")
  }
}

class OAuthClientCredentials(key : LocationApiKey, secret : String) {

  /** Value of the "API Key". */
  val API_KEY = key.getKey

  /** Value of the "API Secret". */
  val API_SECRET = secret

  def errorIfNotSpecified {
    if (API_KEY.startsWith("Enter ") || API_SECRET.startsWith("Enter ")) {
      System.out.println(
        "Enter API Key and API Secret from "
          + " into API_KEY and API_SECRET ")
      System.exit(1)
    }
  }
}
