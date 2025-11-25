<%@ page contentType="text/html;charset=UTF-8" %>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Environment Surveillance</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; background: #f8fafc; color: #0f172a; }
        header { background: linear-gradient(90deg, #0f172a, #1d4ed8); color: #fff; padding: 16px 28px; display: flex; justify-content: space-between; align-items: center; }
        header h1 { margin: 0; font-size: 20px; letter-spacing: 0.5px; }
        main { padding: 24px 28px; }
        .actions { display: flex; gap: 12px; align-items: center; margin-bottom: 18px; flex-wrap: wrap; }
        .btn { background: #1d4ed8; color: #fff; border: none; border-radius: 6px; padding: 10px 14px; cursor: pointer; font-weight: 600; }
        .btn:hover { background: #163fa8; }
        .card { background: #fff; border-radius: 10px; box-shadow: 0 8px 24px rgba(15,23,42,0.08); padding: 16px 18px; }
        table { width: 100%; border-collapse: collapse; margin-top: 12px; }
        th, td { padding: 10px; border-bottom: 1px solid #e2e8f0; text-align: left; }
        th { background: #f1f5f9; font-size: 13px; letter-spacing: 0.3px; }
        .msg { margin-top: 10px; padding: 10px; border-radius: 8px; }
        .msg.ok { background: #ecfdf3; color: #166534; border: 1px solid #a7f3d0; }
        .msg.err { background: #fef2f2; color: #991b1b; border: 1px solid #fecdd3; }
        .empty { color: #475569; margin: 12px 0; }
        .meta { font-size: 13px; color: #475569; margin-top: 6px; }
    </style>
</head>
<body>
<header>
    <h1>Environment Surveillance</h1>
    <form action="/logout" method="post">
        <button class="btn" type="submit">Logout</button>
    </form>
</header>

<main>
    <div class="actions">
        <form action="/sync" method="post">
            <button class="btn" type="submit">Sync now</button>
        </form>
        <div class="meta">Pulls stored readings from the Nano via the ESP01 gateway.</div>
    </div>

    <c:if test="${not empty flashMessage}">
        <div class="msg ok">${flashMessage}</div>
    </c:if>
    <c:if test="${not empty flashError}">
        <div class="msg err">${flashError}</div>
    </c:if>

    <div class="card">
        <h2 style="margin:0 0 8px 0;">Latest readings</h2>
        <c:if test="${empty readings}">
            <div class="empty">No readings yet. Hit "Sync now" after the node has sent data.</div>
        </c:if>
        <c:if test="${not empty readings}">
            <table>
                <thead>
                <tr>
                    <th>ID</th>
                    <th>Sensor</th>
                    <th>Raw value</th>
                    <th>Recorded at</th>
                </tr>
                </thead>
                <tbody>
                <c:forEach items="${readings}" var="r">
                    <tr>
                        <td>${r.id}</td>
                        <td>${r.sensor}</td>
                        <td>${r.rawValue}</td>
                        <td>${r.recordedAt}</td>
                    </tr>
                </c:forEach>
                </tbody>
            </table>
        </c:if>
    </div>
</main>
</body>
</html>
