Responsive Favicons
---------------------

This module adds the favicons generated by http://realfavicongenerator.net/ to
your site. The responsive name comes from the fact that many devices are
catered for including iPhone, Android, iPad, other tablets and dektops.

CONTENTS OF THIS FILE
---------------------

 - Introduction
 - Tested
 - Known Issues
 - Special Thanks
 - Requirements
 - Installation
 - Coming From Drupal?
 - Usage
 - License
 - Credits
 - Maintainers

TESTED
-----

This module has been manually tested successfully creating favicons in Backdrop 1.2 on a MAMP localhost.

Following the basic instructions worked for the basic purpose of this module, but have not tested all options.

KNOWN ISSUES
---------------------

Ported as is, this module classifies the icons as "missing" and does not print the HTML for them.  However, if you remove this check, enabling the module to print the links to all the files/favicons images, they seem to print out just fine.  The one porting this module has enough to do elsewhere that using this module "as is" works for them, and if you would like to fix it or make it work "the intended way" you are more than welcome to post patches or become the maintainer or edit/fork your local copy.  You are in control!

SPECIAL THANKS
--------------

This entire module was spawned from a blog post on the topic located at
http://www.pixelite.co.nz/article/adding-apple-android-favicons-drupal

REQUIREMENTS
------------

* Favicon files generated by http://realfavicongenerator.net/
* HTML code generated by http://realfavicongenerator.net/

INSTALLATION
------------

Install this module using the official Backdrop CMS instructions at https://backdropcms.org/guide/modules


COMING FROM DRUPAL?
-------------------

responsive_favicons_preprocess_page instead of hook_page_alter

PERMISSIONS
------------

Administer responsive favicons


USAGE
-----

1. Generate the favicons and the HTML code on http://realfavicongenerator.net/

   You will need a 260x260px PNG file with appropriate transparency already
   created. Smaller files can be uploaded but these will produce inferior
   results.

   When asked for the "path" in realfavicongenerator.net select "I will place
   favicon files (favicon.ico, apple-touch-icon.png, etc.) at the root of my web
   site. Recommended." - this is because Drupal will rewrite the URLs based on
   your configuration of the module for you.

3. Go to /admin/config/user-interface/responsive_favicons to configure the
   module.

   Here you will need to paste in the HTML provided by realfavicongenerator.net
   and upload the zip file they provided you with.

4. [optional -- not tested yet on Backdrop] You can also allow this module to take over common paths like:

   * favicon.ico
   * apple-touch-icon.png
   * apple-touch-icon-precomposed.png
   * browserconfig.xml

   In order to do this you need to:

   1) Comment out the line

   #RewriteCond %{REQUEST_URI} !=/favicon.ico

   in the Drupal root .htaccess file.

   2) Alter the variable 404_fast_paths_exclude in your settings.php file:

   $conf['404_fast_paths_exclude'] = '/\/(?:styles)\/|favicon\.ico|apple-touch-icon(?:-precomposed)?\.png|browserconfig\.xml/';

You site will now output the required metadata for the favicons to work. You can
always check the status report page to find out if your site has been correctly
configured.

Note. The 'favicon' module duplicates some of the functionality of responsive
favicons, so it is preferred that you only install one of the two modules.

LICENSE
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.

CREDITS
-----------

Supporting organizations:
Catalyst IT
Initial module development sponsorship

This module is based on the Responsive Icons module for Drupal, originally written and maintained by a large number of contributors, including:

- wiifm <https://www.drupal.org/u/wiifm>
- liuba <https://www.drupal.org/u/liuba>

MAINTAINERS
-----------

- seeking

Ported to Backdrop by:

 - biolithic <https://github.com/biolithic>
