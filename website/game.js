// Establish a WebSocket connection to the server
const socket = new WebSocket('ws://localhost:9002');

// Elements from the DOM
const outputElement = document.getElementById('output');
const commandInput = document.getElementById('command-input');
const sendButton = document.getElementById('send-button');
const popup = document.getElementById("popup");
const registerForm = document.getElementById("registerForm");

// Function to append a message to the output div
function appendOutput(message) {
    const messageElement = document.createElement('div');
    messageElement.textContent = message;
    outputElement.appendChild(messageElement);
    outputElement.scrollTop = outputElement.scrollHeight;
}

// Event listener for the send button
sendButton.addEventListener('click', () => {
    const command = commandInput.value.trim();
    if (command) {
        appendOutput("Me: " + command);
        if (command.substr(0,3) == "msg") {
            const message = {
                type: 2,
                data: command.substr(4)
            };
            socket.send(JSON.stringify(message));
        } else {
        const message = {
            type: 1,
            data: command
        };
        socket.send(JSON.stringify(message));
        }
        commandInput.value = '';
    }
});

// Event listener for pressing Enter in the input field
commandInput.addEventListener('keypress', (event) => {
    if (event.key === 'Enter') {
        sendButton.click();
    }
});

// WebSocket event listeners
socket.addEventListener('open', () => {
    appendOutput('Connected to the game server.');

    // Check if user is already registered
    const username = localStorage.getItem('username');
    const uuid = localStorage.getItem('UUID');
    if (username && uuid) {
        const message = {
            type: 0,
            username: username,
            uuid: uuid
        };
        socket.send(JSON.stringify(message));
    } else {
        popup.style.display = "block";
    }
});

socket.addEventListener('message', (event) => {
    const message = JSON.parse(event.data);
    const username = localStorage.getItem('username');
    const uuid = localStorage.getItem('UUID');

    // Check if the message is relevant to the logged-in user
    if (message.user && (message.user.username !== username || message.user.uuid !== uuid)) {
        return; // Ignore messages not relevant to "ME"
    }

    if (message.type === -1) {
        if (message.success) {
            localStorage.setItem('username', message.username);
            localStorage.setItem('UUID', message.uuid);
            popup.style.display = "none";
            registerForm.reset();
            appendOutput('Registration successful.');
        } else {
            alert("Registration failed. Please try again.");
        }
    } else if (message.type === 0) {
        if (message.success) {
            popup.style.display = "none";
            appendOutput('Login successful.');
        } else {
            alert("Login failed. Please register.");
            popup.style.display = "block";
        }
    } else if (message.type === 99) {
        
        appendOutput(message.data);
    } 
});

socket.addEventListener('close', () => {
    appendOutput('Disconnected from the game server.');
});

socket.addEventListener('error', (error) => {
    console.error('WebSocket Error:', error);
    appendOutput('WebSocket error occurred.');
});

// Handle registration form submission
registerForm.addEventListener("submit", (event) => {
    event.preventDefault();
    const username = document.getElementById("username").value.trim();
    if (username) {
        const message = {
            type: -1,
            username: username
        };
        socket.send(JSON.stringify(message));
    } else {
        alert("Username cannot be empty");
    }
});