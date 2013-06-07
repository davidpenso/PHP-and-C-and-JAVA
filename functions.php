<?php
/**
 * Created by JetBrains PhpStorm.
 * User: david
 * Date: 6/7/13
 * Time: 3:48 PM
 * To change this template use File | Settings | File Templates.
 */


function login($username, $password)
{


    $ok = mysql_query('select * from userpass where username = $username and password = md5($password)  ');

    if ($ok == true) {
        return true;
    } else {
        return false;
    }
}