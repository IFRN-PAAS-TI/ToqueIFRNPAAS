import HorarioToque from "./horario.js"
import EditaHorario from "./edita_horario.js"

export default {
    name: 'Horarios',
    data() {
        return {
            index: -1,
            horario: null,
            editar: true,
            erro_msg: "",
            erro: false
        }
    },
    methods: {
        atualizarHorario(hora, minuto, tempo_som, tempo_pausa, repeticao) {
            horario = new HorarioToque(hora, minuto, tempo_som, tempo_pausa, repeticao)
        }
    },
    template: `
    <div id="edita_horario">
        <a href="" v-on:click="this.$parent.deslogar()">Logout</a>
        <h1 v-if="this.editar">Editar horário das {{ this.horario.hora }}:{{ String(this.horario.minuto).padStart(2,'0') }}</h1>
        <h1 v-else>Novo horário</h1>
        <p v-if="this.erro">{{ this.erro_msg }}</p>
        <form>  
        <table>  
        <tr>  
            <td>Hora de toque:</td>  
            <td><input type="text" id="hora_de_toque" placeholder="Formato 00:00"></td>  
        </tr>  
        </table>

    </div>`
}
