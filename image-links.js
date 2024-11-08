// Changes all image tags to clickable links if the source image is
// significantly larger than the size on screen
function afterImageLoaded(img, callback) {
  if (img.complete) {
    callback(img);
  } else {
    img.onload = function() {
      callback(img);
    }
  }
}

for (const img of document.getElementsByTagName("img")) {
  afterImageLoaded(img, function(img) {
    var w = img.width;
    if (w > 100 && img.naturalWidth > w * 1.5) {
      var link = document.createElement("a");
      link.href = img.src;
      img.parentElement.insertBefore(link, img);
      img.parentElement.removeChild(img);
      img.style.cursor = "zoom-in";
      link.appendChild(img);
    }
  });
}
