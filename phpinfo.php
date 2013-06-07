<?php
/**
 * Created by JetBrains PhpStorm.
 * User: david
 * Date: 6/3/13
 * Time: 5:16 PM
 * To change this template use File | Settings | File Templates.
 */

$arra = array('limango' => 'https://www.limango.com.tr/',
                'arabulvar' =>  'http://www.arabulvar.com/',

);

foreach($arra as $site => $url){
    echo "<li><a href= '$url'> $site</a></li>";
}






?>
