
import QtQuick 2.0
import Enginio 1.0

Rectangle {
    //! [client]
    Enginio {
        id: client
        backendId: "YOUR_BACKEND_ID" // from Enginio Dashboard
        backendSecret: "YOUR_BACKEND_SECRET" // from Enginio Dashboard
    }
    //! [client]

    //! [client-signals]
    Enginio {
        onFinished: console.log("Engino request finished." + reply.data)
        onError: console.log("Enginio error " + reply.errorCode + ": " + reply.errorString)
    }
    //! [client-signals]

    //! [client-query]
    Enginio {
        Component.onCompleted: query({objectType: "objects.image",})
    }
    //! [client-query]
}
