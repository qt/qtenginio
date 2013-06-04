.pragma library

function contentTypeFromFilePath(path)
{
    var extension = "";
    var pathParts = path.split("/");
    if (pathParts.length > 0) {
        var fileName = pathParts[pathParts.length - 1];
        var fileParts = fileName.split(".");
        if (fileParts.length > 1) {
            extension = fileParts[fileParts.length - 1];
        }
    }

    switch (extension) {
    case "png":
        return "image/png";
    case "jpg":
    case "jpeg":
        return "image/jpeg";
    default:
        return "";
    }
}

function sizeStringFromFile(fileData)
{
    var str = [];
    if (fileData && fileData.fileSize) {
        str.push("Size: ");
        str.push(fileData.fileSize);
        str.push(" bytes");
    }
    return str.join("");
}

function timeStringFromFile(fileData)
{
    var str = [];
    if (fileData && fileData.createdAt) {
        var date = new Date(fileData.createdAt);
        if (date) {
            str.push("Uploaded: ");
            str.push(date.toDateString());
            str.push(" ");
            str.push(doubleDigitNumber(date.getHours()));
            str.push(":");
            str.push(doubleDigitNumber(date.getMinutes()));
        }
    }
    return str.join("");
}

function doubleDigitNumber(number)
{
    if (number < 10)
        return "0" + number;
    return number;
}
