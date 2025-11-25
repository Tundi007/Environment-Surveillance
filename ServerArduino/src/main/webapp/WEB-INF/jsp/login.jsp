<%@ page contentType="text/html;charset=UTF-8" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Environment Surveillance - Login</title>
    <style>
        body { font-family: Arial, sans-serif; background: #f4f6f8; display: flex; align-items: center; justify-content: center; height: 100vh; margin: 0; }
        .card { background: #fff; padding: 24px 28px; border-radius: 10px; box-shadow: 0 6px 24px rgba(0,0,0,0.08); width: 320px; }
        h1 { margin-top: 0; font-size: 20px; text-align: center; }
        label { display: block; font-weight: bold; margin-top: 12px; }
        input { width: 100%; padding: 10px; margin-top: 6px; border: 1px solid #cfd4da; border-radius: 6px; font-size: 14px; }
        button { width: 100%; margin-top: 18px; padding: 10px; background: #1f6feb; color: #fff; border: none; border-radius: 6px; cursor: pointer; font-size: 15px; }
        button:hover { background: #1857bb; }
        .msg { margin-top: 12px; color: #b00020; text-align: center; }
        .notice { margin-top: 12px; color: #1f6feb; text-align: center; }
    </style>
</head>
<body>
<div class="card">
    <h1>Environment Surveillance</h1>
    <form method="post" action="/login">
        <label for="username">Username</label>
        <input type="text" id="username" name="username" required autofocus>

        <label for="password">Password</label>
        <input type="password" id="password" name="password" required>

        <button type="submit">Sign in</button>
    </form>

    <c:if test="${not empty param.error}">
        <div class="msg">Invalid credentials</div>
    </c:if>
    <c:if test="${not empty param.logout}">
        <div class="notice">Logged out</div>
    </c:if>
</div>
</body>
</html>
