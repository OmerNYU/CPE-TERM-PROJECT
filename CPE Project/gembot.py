import streamlit as st # type: ignore
import vertexai
from vertexai.generative_models import GenerativeModel

# Initialize Vertex AI
PROJECT_ID = "unified-atom-441618-q6"
vertexai.init(project=PROJECT_ID, location="us-central1")

# Set up the Gemini model with a calming persona
model = GenerativeModel(
    "gemini-1.5-flash-002",
    system_instruction="You are an empathetic friend and a calming meditation instructor."
)

# Define chatbot functions

def generate_response(prompt):
    try:
        response = model.generate_content(prompt)
        return response.text
    except Exception as e:
        st.error(f"Error generating response: {e}")
        return "I'm having trouble understanding. Can you try rephrasing?"

def generate_meditation_script(type="deep breathing", duration=5, user_name="friend"):
    prompt = (f"Create a {duration}-minute guided meditation focusing on {type}. "
              f"The script should be calming and reassuring, emphasizing relaxation and presence. "
              f"Address the user as '{user_name}'.")
    return generate_response(prompt)

def provide_stress_advice():
    prompt = "Provide advice on managing stress and anxiety in a gentle, understanding tone."
    return generate_response(prompt)

def generate_empathetic_response(emotion):
    prompt = f"Offer a supportive and empathetic response for someone feeling {emotion}."
    return generate_response(prompt)

# Streamlit UI
st.title("Meditation & Wellness Chatbot")
st.write("Welcome! I'm here to help with meditation, stress advice, or just to chat.")

# Initialize chat history and session state variables
if "chat_history" not in st.session_state:
    st.session_state.chat_history = []

# Handle user input and response generation
user_input = st.text_input("How can I help you today?", key="user_input")

if st.button("Send") and user_input:
    # Check user intent and respond accordingly
    if "meditation" in user_input.lower():
        # Prompt for meditation details if not already set
        if "meditation_type" not in st.session_state:
            st.session_state.meditation_type = st.text_input("What type of meditation? (e.g., deep breathing, body scan)")
            st.session_state.meditation_duration = st.number_input("Duration in minutes", min_value=1, max_value=60, value=5)
        else:
            # Generate meditation script
            response = generate_meditation_script(type=st.session_state.meditation_type, duration=int(st.session_state.meditation_duration))
            st.session_state.chat_history.append(("User", user_input))
            st.session_state.chat_history.append(("Chatbot", response))
            # Clear temporary variables
            del st.session_state.meditation_type
            del st.session_state.meditation_duration

    elif "stress" in user_input.lower():
        # Generate stress advice response
        response = provide_stress_advice()
        st.session_state.chat_history.append(("User", user_input))
        st.session_state.chat_history.append(("Chatbot", response))

    elif "feeling" in user_input.lower():
        # Extract emotion from user input
        emotion = user_input.lower().split("feeling")[-1].strip()
        if emotion:
            response = generate_empathetic_response(emotion)
            st.session_state.chat_history.append(("User", user_input))
            st.session_state.chat_history.append(("Chatbot", response))

    else:
        # General response for other input
        response = generate_response(user_input)
        st.session_state.chat_history.append(("User", user_input))
        st.session_state.chat_history.append(("Chatbot", response))

# Display conversation history
for sender, message in st.session_state.chat_history:
    if sender == "User":
        st.write(f"**You:** {message}")
    else:
        st.write(f"**Chatbot:** {message}")
