<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<html>
<head>
    <title>Climate Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
</head>
<body>
<h2>Climate Dashboard</h2>

<canvas id="tempChart" width="800" height="200"></canvas>
<canvas id="humChart" width="800" height="200"></canvas>
<canvas id="gasChart" width="800" height="200"></canvas>

<button id="summaryBtn">Generate Summary</button>
<pre id="summaryBox"></pre>

<script>
    const labels = ${labels};
    const temps = ${temps};
    const hums = ${hums};
    const gases = ${gases};

    function mkChart(ctxId, label, data, color) {
        const ctx = document.getElementById(ctxId);
        new Chart(ctx, {
            type: 'line',
            data: {
                labels: labels,
                datasets: [{
                    label: label,
                    data: data,
                    fill: false,
                    tension: 0.2
                }]
            },
            options: {
                scales: {
                    x: { display: true },
                    y: { display: true }
                }
            }
        });
    }

    mkChart('tempChart','Temperature', temps);
    mkChart('humChart','Humidity', hums);
    mkChart('gasChart','Gas', gases);

    document.getElementById('summaryBtn').addEventListener('click', async () => {
        const res = await fetch('/api/summary', { method: 'POST' });
        const json = await res.json();
        document.getElementById('summaryBox').innerText = json.summary || JSON.stringify(json, null, 2);
    });
</script>
</body>
</html>
