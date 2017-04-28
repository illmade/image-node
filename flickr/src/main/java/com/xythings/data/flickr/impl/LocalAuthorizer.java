package com.xythings.data.flickr.impl;

import com.google.api.client.auth.oauth.OAuthAuthorizeTemporaryTokenUrl;
import com.google.api.client.util.Preconditions;

import java.awt.Desktop;
import java.awt.Desktop.Action;
import java.io.IOException;
import java.net.URI;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * OAuth 2.0 authorization code flow for an installed Java application that persists end-user
 * credentials.
 *
 * <p>
 * Implementation is thread-safe.
 * </p>
 *
 * @since 1.11
 * @author Yaniv Inbar
 */
public class LocalAuthorizer {

    /** Verification code receiver. */
    private final LocalServiceReceiver receiver;

    private static final Logger LOGGER =
            Logger.getLogger(LocalAuthorizer.class.getName());

    /**
     * @param receiver verification code receiver
     */
    public LocalAuthorizer(LocalServiceReceiver receiver) {
        this.receiver = Preconditions.checkNotNull(receiver);
    }

    /**
     * Authorizes the installed application to access user's protected data.
     *
     * @return credential
     */
    public String authorize(OAuthAuthorizeTemporaryTokenUrl temporaryToken) throws IOException {
        try {
            onAuthorization(temporaryToken);
            // receive authorization code
            String code = receiver.waitForCode();
            return code;
        } finally {
            receiver.stop();
        }
    }

    /**
     * Handles user authorization by redirecting to the OAuth 2.0 authorization server.
     *
     * <p>
     * Default implementation is to call {@code browse(authorizationUrl.build())}. Subclasses may
     * override to provide optional parameters such as the recommended state parameter. Sample
     * implementation:
     * </p>
     *
     * <pre>
     &#64;Override
     protected void onAuthorization(AuthorizationCodeRequestUrl authorizationUrl) throws IOException {
     authorizationUrl.setState("xyz");
     super.onAuthorization(authorizationUrl);
     }
     * </pre>
     *
     * @throws IOException I/O exception
     */
    protected void onAuthorization(OAuthAuthorizeTemporaryTokenUrl temporaryToken) throws IOException {
        browse(temporaryToken.build());
    }

    /**
     * Open a browser at the given URL using {@link Desktop} if available, or alternatively output the
     * URL to {@link System#out} for command-line applications.
     *
     * @param url URL to browse
     */
    public static void browse(String url) {
        Preconditions.checkNotNull(url);
        // Ask user to open in their browser using copy-paste
        System.out.println("Please open the following address in your browser:");
        System.out.println("  " + url);
        // Attempt to open it in the browser
        try {
            if (Desktop.isDesktopSupported()) {
                Desktop desktop = Desktop.getDesktop();
                if (desktop.isSupported(Action.BROWSE)) {
                    System.out.println("Attempting to open that address in the default browser now...");
                    desktop.browse(URI.create(url));
                }
            }
        } catch (IOException e) {
            LOGGER.log(Level.WARNING, "Unable to open browser", e);
        } catch (InternalError e) {
            // A bug in a JRE can cause Desktop.isDesktopSupported() to throw an
            // InternalError rather than returning false. The error reads,
            // "Can't connect to X11 window server using ':0.0' as the value of the
            // DISPLAY variable." The exact error message may vary slightly.
            LOGGER.log(Level.WARNING, "Unable to open browser", e);
        }
    }

}