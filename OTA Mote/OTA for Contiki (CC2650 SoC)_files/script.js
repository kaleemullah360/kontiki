// Window Scroll
var windowScroll = function () {
    $(window).scroll(function () {
        //  (1) Decide whether or not to move title bar
        var scrollPos = $(this).scrollTop();

        if ($(window).scrollTop() > 70) {
            $('.site-header').addClass('site-header-nav-scrolled');
        } else {
            $('.site-header').removeClass('site-header-nav-scrolled');
        }

        /*if ($(window).scrollTop() < 420) {
          $( $("#post-directory-spy .nav li")[0] ).addClass("active");
        }*/

        //  (2)
        update_header_text();
    });
};

var update_header_text = function() {
  if ( $(".nav > li.active").length/* && $(window).scrollTop() > 400*/ ) {
    //$("#site-header-brand").addClass("in-article").removeAttr("href");
  } else {
    $("#site-header-brand").removeClass("in-article").attr("href", "/");
    $("#title-text").text( window.site_title );
  }
};

$( document ).ready(function() {
    window.site_title = $("#title-text").text();
    $( $("#post-directory-spy .nav li")[0] ).addClass("active");
    windowScroll();

    // Find all YouTube videos
    var $allVideos = $("iframe.youtube"),
        // The element that is fluid width
        $fluidEl = $(".content");

    // Figure out and save aspect ratio for each video
    $allVideos.each(function() {
      $(this)
        .data('aspectRatio', this.height / this.width)
        // and remove the hard coded width/height
        .removeAttr('height')
        .removeAttr('width');
    });

    // When the window is resized
    $(window).resize(function() {
      // Resize all videos according to their own aspect ratio
      $allVideos.each(function() {
        var $el = $(this);
        var newWidth = $el.parent().width();
        console.log( newWidth );
        $el
          .width(newWidth)
          .height(newWidth * $el.data('aspectRatio'));

      });
    // Kick off one resize to fix all videos on page load
    }).resize();

    $('#post-directory-spy').on('activate.bs.scrollspy', function () {
      var new_text = "";
      var breadcrumb_flag = false;
      _.each( $(".nav li.active > a"), function(elem) {
        elem = $(elem);
        var text = elem.text();
        var dest = elem.attr("href");
        if (breadcrumb_flag) {
          new_text += " > ";
        } else {
          breadcrumb_flag = true;
        }

        new_text += "<a href=\"" + dest + "\">";
        new_text += text;
        new_text += "</a>";
      });
      //var new_text = $(_.last( $(".nav li.active > a") )).text();
      $("#title-text").html( new_text );
      $("#site-header-brand").addClass("in-article");
    });
});
