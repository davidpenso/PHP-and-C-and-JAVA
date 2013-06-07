<?php
/**
 * Created by JetBrains PhpStorm.
 * User: david
 * Date: 6/6/13
 * Time: 9:34 AM
 * To change this template use File | Settings | File Templates.
 */


session_start();

define(USERNAME,'david');
define(PASSWORD,'1234');

if ($_SERVER[REQUEST_METHOD]== 'POST'){

    $username = $_POST["username"];
    $password = $_POST["password"];

    if($username== USERNAME && $password == PASSWORD ){

        $_SESSION["username"] = $username;
        header("Location: hackerNews.php");
    }
    else{
        $stat = 'incorrect password';
    }

}

?>

<!doctype html>
<html>

<body>


<h1> Login</h1>

<form action="login.php" method='POST' >
<ul>
    <li>
        <label for="username" > Username: </label>
        <input type="text" name="username">
    </li>

    <li>
         <label for="password" > Password:</label>
         <input type="password" name="password">

    </li>

    <li>
        <input type="submit" value="Login" name="LoginForm">

    </li>
</ul>
</form>
<?php if(isset($stat)): ?>
<p> <?php echo $stat;   ?> </p>
<?php  endif;      ?>


</body>


</html>