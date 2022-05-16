export default {
    name: 'Login',
    data() {
        return {
            input: {
                username: "",
                password: "",
                statustext: ""
            },
        }
    },
    methods: {
        login() {
            if(this.input.username != "" && this.input.password != "") {
                if(this.input.username == this.$parent.mockAccount.username && this.input.password == this.$parent.mockAccount.password) {
                    this.$parent.authenticated = true;
                } else {
                    this.input.statustext = "Usuário ou senha incorretos.";
                }
            } else {
                this.input.statustext = "É necessário informar um usuário e uma senha.";
            }
        },
    },
    template: `
        <div id="login">
            <h1>Autenticar-se</h1>
            <p>{{ input.statustext }}</p>
            <input type="text" name="username" v-model="input.username" placeholder="Usuário" />
            <input type="password" name="password" v-model="input.password" placeholder="Senha" />
            <button type="button" v-on:click="login()">Entrar</button>
        </div>`
}
